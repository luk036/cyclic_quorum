/**
 * RL implementation for difference cover with multi-threading
 *
 * Policy gradient (REINFORCE) with entropy regularization,
 * flat array weights, proper backpropagation, local gradient
 * accumulation, and batched network updates.
 *
 * Concurrency model: the shared weights are guarded by one mutex, and each
 * worker keeps a private copy that it refreshes once per gradient batch. The
 * hot forward/backward loops therefore run on plain floats with no atomics and
 * no shared state, so the program has no data races and no undefined behaviour.
 * Batches from different workers are applied to the shared copy in arbitrary
 * order, which keeps the Hogwild-style noise but within well-defined semantics.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>

// Neural Network Constants
constexpr int HIDDEN_SIZE1 = 256;
constexpr int HIDDEN_SIZE2 = 128;
constexpr float LEARNING_RATE = 0.01f;
constexpr float GAMMA = 0.98f;
constexpr int MAX_EPISODES = 1000000000;
constexpr int NUM_THREADS = 10;
constexpr float ENTROPY_BETA = 0.01f;
constexpr int MAX_N = 1000000;  // Upper bound on N (keeps int size arithmetic safe)
constexpr int LOCAL_UPDATE_INTERVAL = 5;

struct ForwardResult {
    std::vector<float> z1_pre;
    std::vector<float> z1;
    std::vector<float> z2_pre;
    std::vector<float> z2;
    std::vector<float> z3;
};

struct StepData {
    std::vector<float> state;
    std::vector<float> probs;
    int action;
    float reward;
};

// The weights live in two places:
//  - one shared copy owned by PolicyNetwork and touched only under its mutex;
//  - one private copy per worker, refreshed from the shared copy once per batch.
// Workers run forward()/backward() on their private copy with plain floats, so
// the hot loops stay vectorisable and no two threads ever touch the same object
// at the same time: there is no data race and no undefined behaviour.
struct Weights {
    int inputSize;
    int outputSize;
    std::vector<float> W1, W2, W3;
    std::vector<float> b1, b2, b3;

    Weights(int inSize, int outSize)
        : inputSize(inSize),
          outputSize(outSize),
          W1(HIDDEN_SIZE1 * inSize),
          W2(HIDDEN_SIZE2 * HIDDEN_SIZE1),
          W3(outSize * HIDDEN_SIZE2),
          b1(HIDDEN_SIZE1, 0.0f),
          b2(HIDDEN_SIZE2, 0.0f),
          b3(outSize, 0.0f) {}

    // Xavier initialisation of the weight matrices; biases start at zero.
    void InitializeXavier() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        auto xavier = [&](int in, int out) {
            return std::sqrt(6.0f / (in + out)) * dist(rng);
        };

        for (int r = 0; r < HIDDEN_SIZE1; ++r)
            for (int c = 0; c < inputSize; ++c)
                W1[r * inputSize + c] = xavier(inputSize, HIDDEN_SIZE1);

        for (int r = 0; r < HIDDEN_SIZE2; ++r)
            for (int c = 0; c < HIDDEN_SIZE1; ++c)
                W2[r * HIDDEN_SIZE1 + c] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);

        for (int r = 0; r < outputSize; ++r)
            for (int c = 0; c < HIDDEN_SIZE2; ++c)
                W3[r * HIDDEN_SIZE2 + c] = xavier(HIDDEN_SIZE2, outputSize);
    }

    ForwardResult forward(const std::vector<float>& input) const {
        ForwardResult fr;
        int IN = inputSize;
        int H1 = HIDDEN_SIZE1;
        int H2 = HIDDEN_SIZE2;
        int OUT = outputSize;

        fr.z1_pre.resize(H1, 0.0f);
        fr.z1.resize(H1, 0.0f);
        for (int r = 0; r < H1; ++r) {
            float sum = 0.0f;
            for (int c = 0; c < IN; ++c)
                sum += W1[r * IN + c] * input[c];
            sum += b1[r];
            fr.z1_pre[r] = sum;
            fr.z1[r] = std::max(0.0f, sum);
        }

        fr.z2_pre.resize(H2, 0.0f);
        fr.z2.resize(H2, 0.0f);
        for (int r = 0; r < H2; ++r) {
            float sum = 0.0f;
            for (int c = 0; c < H1; ++c)
                sum += W2[r * H1 + c] * fr.z1[c];
            sum += b2[r];
            fr.z2_pre[r] = sum;
            fr.z2[r] = std::max(0.0f, sum);
        }

        fr.z3.resize(OUT, 0.0f);
        for (int r = 0; r < OUT; ++r) {
            float sum = 0.0f;
            for (int c = 0; c < H2; ++c)
                sum += W3[r * H2 + c] * fr.z2[c];
            sum += b3[r];
            fr.z3[r] = sum;
        }
        return fr;
    }

    void backward(const ForwardResult& fr, const std::vector<float>& input,
                  const std::vector<float>& probs, int action,
                  float return_val, float entropy_beta, float entropy,
                  std::vector<float>& gradW1_acc, std::vector<float>& gradB1_acc,
                  std::vector<float>& gradW2_acc, std::vector<float>& gradB2_acc,
                  std::vector<float>& gradW3_acc, std::vector<float>& gradB3_acc) const {
        int IN = inputSize;
        int H1 = HIDDEN_SIZE1;
        int H2 = HIDDEN_SIZE2;
        int OUT = outputSize;

        int T = OUT;
        std::vector<float> gradLogits(T);
        for (int i = 0; i < T; ++i) {
            float delta = (i == action) ? 1.0f : 0.0f;
            gradLogits[i] = return_val * (probs[i] - delta)
                          + entropy_beta * probs[i] * (std::log(probs[i] + 1e-10f) + entropy);
        }

        for (int r = 0; r < OUT; ++r) {
            for (int c = 0; c < H2; ++c)
                gradW3_acc[r * H2 + c] += gradLogits[r] * fr.z2[c];
            gradB3_acc[r] += gradLogits[r];
        }

        std::vector<float> dz2(H2, 0.0f);
        for (int c = 0; c < H2; ++c) {
            float sum = 0.0f;
            for (int r = 0; r < OUT; ++r)
                sum += W3[r * H2 + c] * gradLogits[r];
            dz2[c] = sum;
        }
        for (int i = 0; i < H2; ++i)
            dz2[i] *= (fr.z2_pre[i] > 0.0f) ? 1.0f : 0.0f;

        for (int r = 0; r < H2; ++r) {
            for (int c = 0; c < H1; ++c)
                gradW2_acc[r * H1 + c] += dz2[r] * fr.z1[c];
            gradB2_acc[r] += dz2[r];
        }

        std::vector<float> dz1(H1, 0.0f);
        for (int c = 0; c < H1; ++c) {
            float sum = 0.0f;
            for (int r = 0; r < H2; ++r)
                sum += W2[r * H1 + c] * dz2[r];
            dz1[c] = sum;
        }
        for (int i = 0; i < H1; ++i)
            dz1[i] *= (fr.z1_pre[i] > 0.0f) ? 1.0f : 0.0f;

        for (int r = 0; r < H1; ++r) {
            for (int c = 0; c < IN; ++c)
                gradW1_acc[r * IN + c] += dz1[r] * input[c];
            gradB1_acc[r] += dz1[r];
        }
    }

};

// Shared weights plus the two operations that touch them. Workers only ever
// read the shared copy through snapshot() and modify it through update(), both
// of which hold the mutex, so the concurrent access below is well-defined.
class PolicyNetwork {
  private:
    mutable std::mutex networkMutex;
    Weights shared;

  public:
    PolicyNetwork(int inSize, int outSize) : shared(inSize, outSize) {
        shared.InitializeXavier();
    }

    // Copy the shared weights into a worker's private copy. One memcpy per
    // batch; the lock is held only for the length of the copy.
    void snapshot(Weights& local) const {
        std::lock_guard<std::mutex> lock(networkMutex);
        std::copy(shared.W1.begin(), shared.W1.end(), local.W1.begin());
        std::copy(shared.b1.begin(), shared.b1.end(), local.b1.begin());
        std::copy(shared.W2.begin(), shared.W2.end(), local.W2.begin());
        std::copy(shared.b2.begin(), shared.b2.end(), local.b2.begin());
        std::copy(shared.W3.begin(), shared.W3.end(), local.W3.begin());
        std::copy(shared.b3.begin(), shared.b3.end(), local.b3.begin());
    }

    // Apply one worker's accumulated gradients to the shared weights. Batches
    // from different workers interleave in some order and lost updates are
    // still possible (Hogwild semantics), but the accesses themselves are
    // properly synchronised.
    void update(const std::vector<float>& gradW1, const std::vector<float>& gradB1,
                const std::vector<float>& gradW2, const std::vector<float>& gradB2,
                const std::vector<float>& gradW3, const std::vector<float>& gradB3) {
        std::lock_guard<std::mutex> lock(networkMutex);
        for (size_t i = 0; i < shared.W1.size(); ++i)
            shared.W1[i] -= LEARNING_RATE * gradW1[i];
        for (size_t i = 0; i < shared.b1.size(); ++i)
            shared.b1[i] -= LEARNING_RATE * gradB1[i];
        for (size_t i = 0; i < shared.W2.size(); ++i)
            shared.W2[i] -= LEARNING_RATE * gradW2[i];
        for (size_t i = 0; i < shared.b2.size(); ++i)
            shared.b2[i] -= LEARNING_RATE * gradB2[i];
        for (size_t i = 0; i < shared.W3.size(); ++i)
            shared.W3[i] -= LEARNING_RATE * gradW3[i];
        for (size_t i = 0; i < shared.b3.size(); ++i)
            shared.b3[i] -= LEARNING_RATE * gradB3[i];
    }
};

std::vector<float> softmax(const std::vector<float>& logits) {
    std::vector<float> probs(logits.size());
    float maxLogit = *std::max_element(logits.begin(), logits.end());
    float sumExp = 0.0f;
    for (size_t i = 0; i < logits.size(); ++i) {
        probs[i] = std::exp(logits[i] - maxLogit);
        sumExp += probs[i];
    }
    for (size_t i = 0; i < probs.size(); ++i)
        probs[i] /= sumExp;
    return probs;
}

float computeEntropy(const std::vector<float>& probs) {
    float H = 0.0f;
    for (size_t i = 0; i < probs.size(); ++i)
        H -= probs[i] * std::log(probs[i] + 1e-10f);
    return H;
}

void workerThread(PolicyNetwork& policyNet, int N, int D,
                 std::atomic<int>& episodeCounter, std::atomic<bool>& solutionFound,
                 std::mutex& outputMutex) {
    std::mt19937 gen(std::random_device{}());

    int IN = 2 * N;
    int H1 = HIDDEN_SIZE1;
    int H2 = HIDDEN_SIZE2;

    std::vector<float> state(IN, 0.0f);

    int episodesSinceUpdate = 0;

    std::vector<float> localGradW1(H1 * IN, 0.0f);
    std::vector<float> localGradB1(H1, 0.0f);
    std::vector<float> localGradW2(H2 * H1, 0.0f);
    std::vector<float> localGradB2(H2, 0.0f);
    std::vector<float> localGradW3(N * H2, 0.0f);
    std::vector<float> localGradB3(N, 0.0f);

    // Private copy of the network used for every forward/backward pass. It is
    // refreshed from the shared copy once per gradient batch, so the hot loops
    // run on plain floats with no atomics and no shared state.
    Weights local(IN, N);
    policyNet.snapshot(local);

    while (!solutionFound && episodeCounter < MAX_EPISODES) {
        int episode = episodeCounter++;
        if (episode >= MAX_EPISODES) break;
        episodesSinceUpdate++;

        std::vector<int> chosen(N, 0);
        std::vector<int> chosenList = {0};
        chosen[0] = 1;
        std::vector<int> residues(N, 0);
        residues[0] = 1;

        std::vector<StepData> trajectory;

        for (int step = 0; step < D - 1; ++step) {
            for (int i = 0; i < N; ++i) {
                state[i] = static_cast<float>(chosen[i]);
                state[N + i] = static_cast<float>(residues[i]);
            }

            ForwardResult fr = local.forward(state);

            for (int i = 0; i < N; ++i) {
                if (chosen[i]) fr.z3[i] = -1e9f;
            }

            std::vector<float> probs = softmax(fr.z3);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);

            chosen[action] = 1;
            chosenList.push_back(action);
            int newCovered = 0;
            for (size_t ci = 0; ci < chosenList.size(); ++ci) {
                int idx = chosenList[ci];
                if (idx == action) continue;
                int res1 = (action - idx + N) % N;
                int res2 = (idx - action + N) % N;
                if (!residues[res1]) { residues[res1] = 1; newCovered++; }
                if (!residues[res2]) { residues[res2] = 1; newCovered++; }
            }

            StepData sd;
            sd.state = state;
            sd.probs = std::move(probs);
            sd.action = action;
            sd.reward = static_cast<float>(newCovered);
            trajectory.push_back(std::move(sd));
        }

        bool isSolution = true;
        for (int i = 0; i < N; ++i) {
            if (!residues[i]) { isSolution = false; break; }
        }

        if (isSolution) {
            std::lock_guard<std::mutex> lock(outputMutex);
            solutionFound = true;
            printf("\nSolution found in episode %d:\n", episode);
            for (int i = 0; i < N; ++i) {
                if (chosen[i]) printf("%d ", i);
            }
            printf("\n");
            return;
        }

        int T = static_cast<int>(trajectory.size());
        std::vector<float> returns(T, 0.0f);
        float G = 0.0f;
        for (int t = T - 1; t >= 0; --t) {
            G = GAMMA * G + trajectory[t].reward;
            returns[t] = G;
        }

        float mean = 0.0f, stddev = 0.0f;
        for (int t = 0; t < T; ++t) mean += returns[t];
        mean /= T;
        for (int t = 0; t < T; ++t) stddev += (returns[t] - mean) * (returns[t] - mean);
        stddev = std::sqrt(stddev / T);
        if (stddev < 1e-5f) stddev = 1.0f;
        for (int t = 0; t < T; ++t) returns[t] = (returns[t] - mean) / stddev;

        for (int t = 0; t < T; ++t) {
            ForwardResult fr = local.forward(trajectory[t].state);
            float H = computeEntropy(trajectory[t].probs);
            local.backward(fr, trajectory[t].state, trajectory[t].probs,
                               trajectory[t].action, returns[t], ENTROPY_BETA, H,
                               localGradW1, localGradB1,
                               localGradW2, localGradB2,
                               localGradW3, localGradB3);
        }

        if (episodesSinceUpdate >= LOCAL_UPDATE_INTERVAL) {
            policyNet.update(localGradW1, localGradB1,
                             localGradW2, localGradB2,
                             localGradW3, localGradB3);
            std::fill(localGradW1.begin(), localGradW1.end(), 0.0f);
            std::fill(localGradB1.begin(), localGradB1.end(), 0.0f);
            std::fill(localGradW2.begin(), localGradW2.end(), 0.0f);
            std::fill(localGradB2.begin(), localGradB2.end(), 0.0f);
            std::fill(localGradW3.begin(), localGradW3.end(), 0.0f);
            std::fill(localGradB3.begin(), localGradB3.end(), 0.0f);
            episodesSinceUpdate = 0;
            // Pick up the other workers' updates before computing the next batch.
            policyNet.snapshot(local);
        }
    }
}

void findDifferenceCoverRL(int N, int D) {
    const int inputSize = 2 * N;
    PolicyNetwork policyNet(inputSize, N);

    std::atomic<int> episodeCounter(0);
    std::atomic<bool> solutionFound(false);
    std::mutex outputMutex;

    std::vector<std::thread> threads;
    for (int idx = 0; idx < NUM_THREADS; ++idx) {
        threads.emplace_back(workerThread, std::ref(policyNet), N, D,
                           std::ref(episodeCounter), std::ref(solutionFound),
                           std::ref(outputMutex));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (!solutionFound) {
        printf("No solution found after %d episodes\n", MAX_EPISODES);
    }
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        printf("Usage: diff_cover_rl [n] [d]\n");
        return 1;
    }

    int N = atoi(argv[1]);
    int D = atoi(argv[2]);

    // Bound D and N before the product so allocation-size arithmetic cannot
    // overflow int, and the product itself cannot overflow.
    if (N < 3 || D < 3 || D > N || N > MAX_N ||
        static_cast<long long>(N) > static_cast<long long>(D) * (D - 1) + 1) {
        printf("Invalid parameters: n>=3, d>=3, n<=d*(d-1)+1\n");
        return 1;
    }

    findDifferenceCoverRL(N, D);
    return 0;
}
