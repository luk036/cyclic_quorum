/*
 * Copyright (c) 2019 Joe Sawada
 * RL implementation for difference cover
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>
#include <algorithm>

constexpr int HIDDEN_SIZE1 = 256;
constexpr int HIDDEN_SIZE2 = 128;
constexpr float LEARNING_RATE = 0.01f;
constexpr float GAMMA = 0.99f;
constexpr int MAX_EPISODES = 500000;
constexpr float ENTROPY_BETA = 0.01f;

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

        W1.resize(HIDDEN_SIZE1 * inputSize, 0.0f);
        b1.resize(HIDDEN_SIZE1, 0.0f);
        for (int r = 0; r < HIDDEN_SIZE1; ++r)
            for (int c = 0; c < inputSize; ++c)
                W1[r * inputSize + c] = xavier(inputSize, HIDDEN_SIZE1);

        W2.resize(HIDDEN_SIZE2 * HIDDEN_SIZE1, 0.0f);
        b2.resize(HIDDEN_SIZE2, 0.0f);
        for (int r = 0; r < HIDDEN_SIZE2; ++r)
            for (int c = 0; c < HIDDEN_SIZE1; ++c)
                W2[r * HIDDEN_SIZE1 + c] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);

        W3.resize(outputSize * HIDDEN_SIZE2, 0.0f);
        b3.resize(outputSize, 0.0f);
        for (int r = 0; r < outputSize; ++r)
            for (int c = 0; c < HIDDEN_SIZE2; ++c)
                W3[r * HIDDEN_SIZE2 + c] = xavier(HIDDEN_SIZE2, outputSize);
    }

    ForwardResult forward(const std::vector<float>& input) {
        ForwardResult fr;
        fr.z1_pre.resize(HIDDEN_SIZE1);
        fr.z1.resize(HIDDEN_SIZE1);
        for (int r = 0; r < HIDDEN_SIZE1; ++r) {
            float sum = b1[r];
            for (int c = 0; c < inputSize; ++c)
                sum += W1[r * inputSize + c] * input[c];
            fr.z1_pre[r] = sum;
            fr.z1[r] = std::max(0.0f, sum);
        }

        fr.z2_pre.resize(HIDDEN_SIZE2);
        fr.z2.resize(HIDDEN_SIZE2);
        for (int r = 0; r < HIDDEN_SIZE2; ++r) {
            float sum = b2[r];
            for (int c = 0; c < HIDDEN_SIZE1; ++c)
                sum += W2[r * HIDDEN_SIZE1 + c] * fr.z1[c];
            fr.z2_pre[r] = sum;
            fr.z2[r] = std::max(0.0f, sum);
        }

        fr.z3.resize(outputSize);
        for (int r = 0; r < outputSize; ++r) {
            float sum = b3[r];
            for (int c = 0; c < HIDDEN_SIZE2; ++c)
                sum += W3[r * HIDDEN_SIZE2 + c] * fr.z2[c];
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
        const int H1 = HIDDEN_SIZE1, H2 = HIDDEN_SIZE2, IN = inputSize, OUT = outputSize;

        std::vector<float> gradLogits(OUT, 0.0f);
        for (int i = 0; i < OUT; ++i) {
            float indicator = (i == action) ? 1.0f : 0.0f;
            gradLogits[i] = return_val * (probs[i] - indicator)
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
            if (fr.z2_pre[c] > 0.0f) dz2[c] = sum;
        }

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
            if (fr.z1_pre[c] > 0.0f) dz1[c] = sum;
        }

        for (int r = 0; r < H1; ++r) {
            for (int c = 0; c < IN; ++c)
                gradW1_acc[r * IN + c] += dz1[r] * input[c];
            gradB1_acc[r] += dz1[r];
        }
    }

    void update(const std::vector<float>& gradW1, const std::vector<float>& gradB1,
                const std::vector<float>& gradW2, const std::vector<float>& gradB2,
                const std::vector<float>& gradW3, const std::vector<float>& gradB3) {
        for (size_t i = 0; i < W1.size(); ++i) W1[i] -= LEARNING_RATE * gradW1[i];
        for (size_t i = 0; i < b1.size(); ++i) b1[i] -= LEARNING_RATE * gradB1[i];
        for (size_t i = 0; i < W2.size(); ++i) W2[i] -= LEARNING_RATE * gradW2[i];
        for (size_t i = 0; i < b2.size(); ++i) b2[i] -= LEARNING_RATE * gradB2[i];
        for (size_t i = 0; i < W3.size(); ++i) W3[i] -= LEARNING_RATE * gradW3[i];
        for (size_t i = 0; i < b3.size(); ++i) b3[i] -= LEARNING_RATE * gradB3[i];
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

void findDifferenceCoverRL(int N, int D) {
    const int inputSize = 2 * N;
    PolicyNetwork policyNet(inputSize, N);
    std::mt19937 gen(std::random_device{}());

    for (int episode = 0; episode < MAX_EPISODES; ++episode) {
        std::vector<int> chosen(N, 0);
        std::vector<int> residues(N, 0);
        std::vector<int> chosenList = {0};
        chosen[0] = 1;
        residues[0] = 1;

        std::vector<StepData> trajectory;
        std::vector<float> state(2 * N, 0.0f);

        for (int step = 0; step < D - 1; ++step) {
            std::fill(state.begin(), state.end(), 0.0f);
            for (int i = 0; i < N; ++i) {
                state[i] = static_cast<float>(chosen[i]);
                state[N + i] = static_cast<float>(residues[i]);
            }

            ForwardResult fr = policyNet.forward(state);
            std::vector<float> logits = fr.z3;
            for (int i = 0; i < N; ++i)
                if (chosen[i]) logits[i] = -1e9f;

            std::vector<float> probs = softmax(logits);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);

            chosen[action] = 1;
            chosenList.push_back(action);
            int newCovered = 0;
            for (size_t j = 0; j < chosenList.size() - 1; ++j) {
                int jdx = chosenList[j];
                int res1 = (action - jdx + N) % N;
                int res2 = (jdx - action + N) % N;
                if (!residues[res1]) { residues[res1] = 1; newCovered++; }
                if (!residues[res2]) { residues[res2] = 1; newCovered++; }
            }

            trajectory.push_back({state, probs, action, static_cast<float>(newCovered)});
        }

        bool isSolution = true;
        for (int i = 0; i < N; ++i) {
            if (!residues[i]) { isSolution = false; break; }
        }
        if (isSolution) {
            printf("\nSolution found in episode %d:\n", episode);
            for (int i = 0; i < N; ++i)
                if (chosen[i]) printf("%d ", i);
            printf("\n");
            return;
        }

        std::vector<float> returns(trajectory.size());
        float G = 0.0f;
        for (int t = static_cast<int>(trajectory.size()) - 1; t >= 0; --t) {
            G = GAMMA * G + trajectory[t].reward;
            returns[t] = G;
        }

        float mean = 0.0f, stdev = 0.0f;
        for (float r : returns) mean += r;
        mean /= returns.size();
        for (float r : returns) stdev += (r - mean) * (r - mean);
        stdev = std::sqrt(stdev / returns.size());
        if (stdev < 1e-5f) stdev = 1.0f;
        for (float& r : returns) r = (r - mean) / stdev;

        std::vector<float> gradW1(HIDDEN_SIZE1 * inputSize, 0.0f);
        std::vector<float> gradB1(HIDDEN_SIZE1, 0.0f);
        std::vector<float> gradW2(HIDDEN_SIZE2 * HIDDEN_SIZE1, 0.0f);
        std::vector<float> gradB2(HIDDEN_SIZE2, 0.0f);
        std::vector<float> gradW3(N * HIDDEN_SIZE2, 0.0f);
        std::vector<float> gradB3(N, 0.0f);

        for (size_t t = 0; t < trajectory.size(); ++t) {
            float entropy = 0.0f;
            for (float p : trajectory[t].probs)
                entropy -= p * std::log(p + 1e-10f);

            ForwardResult fr = policyNet.forward(trajectory[t].state);
            policyNet.backward(fr, trajectory[t].state, trajectory[t].probs,
                              trajectory[t].action, returns[t], ENTROPY_BETA, entropy,
                              gradW1, gradB1, gradW2, gradB2, gradW3, gradB3);
        }

        policyNet.update(gradW1, gradB1, gradW2, gradB2, gradW3, gradB3);
    }

    printf("No solution found after %d episodes\n", MAX_EPISODES);
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
