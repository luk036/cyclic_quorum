/**
 * RL implementation for difference cover with multi-threading
 *
 * Policy gradient (REINFORCE) with entropy regularization,
 * flat array weights, proper backpropagation, local gradient
 * accumulation, and batched network updates.
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

class PolicyNetwork {
private:
    int inputSize;
    int outputSize;
    std::vector<float> W1, W2, W3;
    std::vector<float> b1, b2, b3;
    mutable std::mutex networkMutex;

public:
    PolicyNetwork(int inSize, int outSize)
        : inputSize(inSize), outputSize(outSize) {
        InitializeWeights();
    }

    void InitializeWeights() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        auto xavier = [&](int in, int out) {
            return std::sqrt(6.0f / (in + out)) * dist(rng);
        };

        W1.resize(HIDDEN_SIZE1 * inputSize);
        for (int r = 0; r < HIDDEN_SIZE1; ++r)
            for (int c = 0; c < inputSize; ++c)
                W1[r * inputSize + c] = xavier(inputSize, HIDDEN_SIZE1);
        b1.resize(HIDDEN_SIZE1, 0.0f);

        W2.resize(HIDDEN_SIZE2 * HIDDEN_SIZE1);
        for (int r = 0; r < HIDDEN_SIZE2; ++r)
            for (int c = 0; c < HIDDEN_SIZE1; ++c)
                W2[r * HIDDEN_SIZE1 + c] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);
        b2.resize(HIDDEN_SIZE2, 0.0f);

        W3.resize(outputSize * HIDDEN_SIZE2);
        for (int r = 0; r < outputSize; ++r)
            for (int c = 0; c < HIDDEN_SIZE2; ++c)
                W3[r * HIDDEN_SIZE2 + c] = xavier(HIDDEN_SIZE2, outputSize);
        b3.resize(outputSize, 0.0f);
    }

    ForwardResult forward(const std::vector<float>& input) {
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
                          - entropy_beta * probs[i] * (std::log(probs[i] + 1e-10f) + entropy);
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

    void update(const std::vector<float>& gradW1, const std::vector<float>& gradB1,
                const std::vector<float>& gradW2, const std::vector<float>& gradB2,
                const std::vector<float>& gradW3, const std::vector<float>& gradB3) {
        std::lock_guard<std::mutex> lock(networkMutex);
        for (size_t i = 0; i < W1.size(); ++i) W1[i] -= LEARNING_RATE * gradW1[i];
        for (size_t i = 0; i < b1.size(); ++i) b1[i] -= LEARNING_RATE * gradB1[i];
        for (size_t i = 0; i < W2.size(); ++i) W2[i] -= LEARNING_RATE * gradW2[i];
        for (size_t i = 0; i < b2.size(); ++i) b2[i] -= LEARNING_RATE * gradB2[i];
        for (size_t i = 0; i < W3.size(); ++i) W3[i] -= LEARNING_RATE * gradW3[i];
        for (size_t i = 0; i < b3.size(); ++i) b3[i] -= LEARNING_RATE * gradB3[i];
    }

    void getWeights(std::vector<float>& outW1, std::vector<float>& outB1,
                    std::vector<float>& outW2, std::vector<float>& outB2,
                    std::vector<float>& outW3, std::vector<float>& outB3) const {
        std::lock_guard<std::mutex> lock(networkMutex);
        outW1 = W1; outB1 = b1; outW2 = W2; outB2 = b2; outW3 = W3; outB3 = b3;
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

            ForwardResult fr = policyNet.forward(state);

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
            ForwardResult fr = policyNet.forward(trajectory[t].state);
            float H = computeEntropy(trajectory[t].probs);
            policyNet.backward(fr, trajectory[t].state, trajectory[t].probs,
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

    if (N < 3 || D < 3 || N > D * (D - 1) + 1) {
        printf("Invalid parameters: n>=3, d>=3, n<=d*(d-1)+1\n");
        return 1;
    }

    findDifferenceCoverRL(N, D);
    return 0;
}
