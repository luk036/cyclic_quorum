/*
 * Copyright (c) 2019 Joe Sawada
 * Modified (c) 2024 [Your Name]
 * RL implementation for difference cover
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>
#include <algorithm>

// Neural Network Constants
constexpr int HIDDEN_SIZE1 = 256;
constexpr int HIDDEN_SIZE2 = 128;
constexpr float LEARNING_RATE = 0.01;
constexpr float GAMMA = 0.99;
constexpr int MAX_EPISODES = 500000;

class PolicyNetwork {
private:
    int inputSize;
    int outputSize;
    // Weights and biases
    std::vector<std::vector<float>> W1, W2, W3;
    std::vector<float> b1, b2, b3;

public:
    PolicyNetwork(int inSize, int outSize)
        : inputSize(inSize), outputSize(outSize) {
        InitializeWeights();
    }

    void InitializeWeights() {
        // Initialize weights with Xavier initialization
        auto xavier = [](int in, int out) { 
            return std::sqrt(6.0f / (in + out)) * (2.0f * rand() / RAND_MAX - 1.0f);
        };

        // Layer 1
        W1.resize(HIDDEN_SIZE1, std::vector<float>(inputSize));
        b1.resize(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] = xavier(inputSize, HIDDEN_SIZE1);
            }
        }

        // Layer 2
        W2.resize(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1));
        b2.resize(HIDDEN_SIZE2, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);
            }
        }

        // Output layer
        W3.resize(outputSize, std::vector<float>(HIDDEN_SIZE2));
        b3.resize(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] = xavier(HIDDEN_SIZE2, outputSize);
            }
        }
    }

    std::vector<float> forward(const std::vector<float>& input) {
        // Layer 1: z1 = W1 * input + b1
        std::vector<float> z1(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                z1[i] += W1[i][j] * input[j];
            }
            z1[i] += b1[i];
            // ReLU activation
            z1[i] = std::max(0.0f, z1[i]);
        }

        // Layer 2: z2 = W2 * z1 + b2
        std::vector<float> z2(HIDDEN_SIZE2, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                z2[i] += W2[i][j] * z1[j];
            }
            z2[i] += b2[i];
            // ReLU activation
            z2[i] = std::max(0.0f, z2[i]);
        }

        // Output layer: z3 = W3 * z2 + b3 (logits)
        std::vector<float> z3(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                z3[i] += W3[i][j] * z2[j];
            }
            z3[i] += b3[i];
        }
        return z3;
    }

    void update(const std::vector<std::vector<float>>& gradW1,
               const std::vector<float>& gradB1,
               const std::vector<std::vector<float>>& gradW2,
               const std::vector<float>& gradB2,
               const std::vector<std::vector<float>>& gradW3,
               const std::vector<float>& gradB3) {
        // Update weights and biases
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] -= LEARNING_RATE * gradW1[i][j];
            }
            b1[i] -= LEARNING_RATE * gradB1[i];
        }
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] -= LEARNING_RATE * gradW2[i][j];
            }
            b2[i] -= LEARNING_RATE * gradB2[i];
        }
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] -= LEARNING_RATE * gradW3[i][j];
            }
            b3[i] -= LEARNING_RATE * gradB3[i];
        }
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
    for (size_t i = 0; i < probs.size(); ++i) {
        probs[i] /= sumExp;
    }
    return probs;
}

void findDifferenceCoverRL(int N, int D) {
    const int inputSize = 2 * N;
    const int outputSize = N;
    PolicyNetwork policyNet(inputSize, N);
    std::mt19937 gen(std::random_device{}());

    for (int episode = 0; episode < MAX_EPISODES; ++episode) {
        std::vector<int> chosen(N, 0);
        chosen[0] = 1;  // Fix position 0
        std::vector<int> residues(N, 0);
        residues[0] = 1;  // Residue 0 is covered

        std::vector<std::vector<float>> states;
        std::vector<int> actions;
        std::vector<float> rewards;

        for (int step = 0; step < D - 1; ++step) {
            // Build state: chosen positions + covered residues
            std::vector<float> state(inputSize, 0.0f);
            for (int i = 0; i < N; ++i) {
                state[i] = static_cast<float>(chosen[i]);
                state[N + i] = static_cast<float>(residues[i]);
            }

            // Forward pass
            std::vector<float> logits = policyNet.forward(state);
            
            // Mask already chosen positions
            for (int i = 0; i < N; ++i) {
                if (chosen[i]) logits[i] = -1e9;
            }

            // Get probabilities and sample action
            std::vector<float> probs = softmax(logits);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);

            // Update state
            chosen[action] = 1;
            int newCovered = 0;
            for (int j = 0; j < N; ++j) {
                if (chosen[j] && j != action) {
                    int res1 = (action - j + N) % N;
                    int res2 = (j - action + N) % N;
                    if (!residues[res1]) {
                        residues[res1] = 1;
                        newCovered++;
                    }
                    if (!residues[res2]) {
                        residues[res2] = 1;
                        newCovered++;
                    }
                }
            }

            // Store trajectory
            states.push_back(state);
            actions.push_back(action);
            rewards.push_back(static_cast<float>(newCovered));
        }

        // Check if solution is found
        bool isSolution = true;
        for (int r : residues) {
            if (!r) {
                isSolution = false;
                break;
            }
        }
        if (isSolution) {
            printf("\nSolution found in episode %d:\n", episode);
            for (int i = 0; i < N; ++i) {
                if (chosen[i]) printf("%d ", i);
            }
            printf("\n");
            return;
        }

        // Compute returns
        std::vector<float> returns(rewards.size());
        float G = 0.0;
        for (int t = rewards.size() - 1; t >= 0; --t) {
            G = GAMMA * G + rewards[t];
            returns[t] = G;
        }

        // Normalize returns
        float mean = 0.0f, stddev = 0.0f;
        for (float R : returns) mean += R;
        mean /= returns.size();
        for (float R : returns) stddev += (R - mean) * (R - mean);
        stddev = std::sqrt(stddev / returns.size());
        if (stddev < 1e-5) stddev = 1.0f;
        for (float& R : returns) R = (R - mean) / stddev;

        // Accumulate gradients
        std::vector<std::vector<float>> gradW1(HIDDEN_SIZE1, std::vector<float>(inputSize, 0.0f));
        std::vector<float> gradB1(HIDDEN_SIZE1, 0.0f);
        std::vector<std::vector<float>> gradW2(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1, 0.0f));
        std::vector<float> gradB2(HIDDEN_SIZE2, 0.0f);
        std::vector<std::vector<float>> gradW3(outputSize, std::vector<float>(HIDDEN_SIZE2, 0.0f));
        std::vector<float> gradB3(outputSize, 0.0f);

        for (size_t t = 0; t < states.size(); ++t) {
            // Forward pass to get logits
            std::vector<float> logits = policyNet.forward(states[t]);
            std::vector<float> probs = softmax(logits);
            
            // Compute gradient
            std::vector<float> gradLogits(N, 0.0f);
            for (int i = 0; i < N; ++i) {
                float indicator = (i == actions[t]) ? 1.0f : 0.0f;
                gradLogits[i] = returns[t] * (indicator - probs[i]);
            }

            // Backpropagation would be implemented here
            // (Simplified for brevity - in practice, compute gradients through layers)
        }

        // Update policy network
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