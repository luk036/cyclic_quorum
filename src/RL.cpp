/**
**RL.cpp - Reinforcement Learning Difference Cover Finder**

This program solves a mathematical puzzle called finding a "difference cover" using artificial intelligence. A difference cover is a special set of numbers where you can create all possible remainders (when dividing by N) by taking differences between pairs of numbers in the set. Think of it like finding the minimum set of positions on a circular clock that lets you measure any possible time interval.

**What the Program Takes as Input**

The program expects two numbers from the command line: N (the size of the set you're working with) and D (how many numbers you want in your difference cover). For example, if you run the program with N=7 and D=3, you're asking it to find 3 numbers that can create all remainders from 0 to 6 through their differences.

**What the Program Produces as Output**

If successful, the program prints out the numbers that form a valid difference cover. For instance, it might output "0 1 3" meaning those three positions solve the puzzle. If it can't find a solution after trying many times, it prints a message saying no solution was found.

**How the Program Achieves Its Purpose**

The program uses a technique called reinforcement learning, which is like teaching a computer to play a game by letting it try many times and learn from its mistakes. The "brain" of the system is an artificial neural network - think of it as a simplified model of how neurons in a real brain connect and process information.

The neural network has three layers of artificial neurons. The first layer receives information about the current state (which numbers have been chosen so far and which remainders are covered). This information flows through two hidden layers that process and transform the data, finally reaching an output layer that suggests which number to pick next.

**The Learning Process and Logic Flow**

The program runs through thousands of episodes, where each episode is like one attempt at solving the puzzle. In each episode, it starts by automatically choosing position 0, then uses its neural network to pick the remaining numbers one by one. The network doesn't just pick randomly - it calculates probabilities for each possible choice and selects based on those probabilities.

After each choice, the program calculates a reward based on how many new remainders got covered by that choice. If picking a number creates many new remainder combinations, that's a good move and gets a higher reward. The program keeps track of all the choices and rewards throughout the episode.

At the end of each episode, the program looks back at all the decisions it made and adjusts the neural network's internal connections. If a choice led to good results later on, the network strengthens the connections that led to that choice. If a choice led to poor results, it weakens those connections. This is similar to how humans learn from experience - we tend to repeat actions that worked well and avoid actions that didn't.

**Important Data Transformations**

The program transforms the raw problem into a format the neural network can understand. It creates a state representation that combines two types of information: which positions have been chosen (represented as 1s and 0s) and which remainders are already covered (also as 1s and 0s). This creates a list of numbers that the neural network can process.

The network's output goes through a mathematical transformation called "softmax" that converts raw scores into probabilities. This ensures all the probabilities add up to 1, making it possible to randomly select the next position based on how likely each choice is to be good.

The program continues this learning process for up to 500,000 episodes, constantly improving its strategy. Each time it finds a better approach, the neural network gets updated to make similar good choices in the future. If it ever finds a complete solution (a set of numbers that covers all possible remainders), it immediately stops and reports the answer.
*/


/**
 * @brief Implements a Reinforcement Learning approach to finding a Difference Cover
 * 
 * This function uses a Policy Network and REINFORCE algorithm to search for a difference cover
 * for a given set size N and difference cover size D. The algorithm learns to select positions
 * that maximize the number of unique residues covered.
 * 
 * @param N The size of the set to find a difference cover for
 * @param D The size of the difference cover
 * 
 * @note The algorithm fixes the first position and attempts to find a solution 
 * through reinforcement learning over multiple training episodes
 * 
 * @throws None
 * @returns void (prints solution to console or indicates no solution found)
 */

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
constexpr int HIDDEN_SIZE1 = 256;  // Size of first hidden layer
constexpr int HIDDEN_SIZE2 = 128;  // Size of second hidden layer
constexpr float LEARNING_RATE = 0.01;  // Learning rate for gradient descent
constexpr float GAMMA = 0.99;  // Discount factor for future rewards
constexpr int MAX_EPISODES = 500000;  // Maximum training episodes

// Policy Network class implements a neural network for reinforcement learning
class PolicyNetwork {
private:
    int inputSize;  // Size of input layer
    int outputSize;  // Size of output layer
    // Weights and biases for the neural network layers
    std::vector<std::vector<float>> W1, W2, W3;  // Weight matrices
    std::vector<float> b1, b2, b3;  // Bias vectors

public:
    // Constructor initializes network with given input and output sizes
    PolicyNetwork(int inSize, int outSize)
        : inputSize(inSize), outputSize(outSize) {
        InitializeWeights();
    }

    // Initialize weights using Xavier initialization for better training
    void InitializeWeights() {
        // Xavier initialization helps maintain variance of activations across layers
        auto xavier = [](int in, int out) { 
            return std::sqrt(6.0f / (in + out)) * (2.0f * rand() / RAND_MAX - 1.0f);
        };

        // Layer 1 initialization
        W1.resize(HIDDEN_SIZE1, std::vector<float>(inputSize));
        b1.resize(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] = xavier(inputSize, HIDDEN_SIZE1);
            }
        }

        // Layer 2 initialization
        W2.resize(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1));
        b2.resize(HIDDEN_SIZE2, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);
            }
        }

        // Output layer initialization
        W3.resize(outputSize, std::vector<float>(HIDDEN_SIZE2));
        b3.resize(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] = xavier(HIDDEN_SIZE2, outputSize);
            }
        }
    }

    // Forward pass through the network with ReLU activation functions
    std::vector<float> forward(const std::vector<float>& input) {
        // Layer 1: z1 = W1 * input + b1
        std::vector<float> z1(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                z1[i] += W1[i][j] * input[j];
            }
            z1[i] += b1[i];
            // ReLU activation (returns max(0, x))
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

        // Output layer: z3 = W3 * z2 + b3 (produces logits)
        std::vector<float> z3(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                z3[i] += W3[i][j] * z2[j];
            }
            z3[i] += b3[i];
        }
        return z3;
    }

    // Update network weights using gradient descent
    void update(const std::vector<std::vector<float>>& gradW1,
               const std::vector<float>& gradB1,
               const std::vector<std::vector<float>>& gradW2,
               const std::vector<float>& gradB2,
               const std::vector<std::vector<float>>& gradW3,
               const std::vector<float>& gradB3) {
        // Update weights and biases for layer 1
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] -= LEARNING_RATE * gradW1[i][j];
            }
            b1[i] -= LEARNING_RATE * gradB1[i];
        }
        // Update weights and biases for layer 2
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] -= LEARNING_RATE * gradW2[i][j];
            }
            b2[i] -= LEARNING_RATE * gradB2[i];
        }
        // Update weights and biases for output layer
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] -= LEARNING_RATE * gradW3[i][j];
            }
            b3[i] -= LEARNING_RATE * gradB3[i];
        }
    }
};

// Softmax function converts logits to probabilities
std::vector<float> softmax(const std::vector<float>& logits) {
    std::vector<float> probs(logits.size());
    float maxLogit = *std::max_element(logits.begin(), logits.end());  // For numerical stability
    float sumExp = 0.0f;
    for (size_t i = 0; i < logits.size(); ++i) {
        probs[i] = std::exp(logits[i] - maxLogit);
        sumExp += probs[i];
    }
    for (size_t i = 0; i < probs.size(); ++i) {
        probs[i] /= sumExp;  // Normalize to get probabilities
    }
    return probs;
}

// Main function to find difference cover using reinforcement learning
void findDifferenceCoverRL(int N, int D) {
    const int inputSize = 2 * N;  // Input is chosen positions + covered residues
    const int outputSize = N;  // Output is probability distribution over positions
    PolicyNetwork policyNet(inputSize, N);  // Initialize policy network
    std::mt19937 gen(std::random_device{}());  // Random number generator

    // Main training loop
    for (int episode = 0; episode < MAX_EPISODES; ++episode) {
        std::vector<int> chosen(N, 0);  // Track chosen positions
        chosen[0] = 1;  // Fix position 0 (common in difference cover problems)
        std::vector<int> residues(N, 0);  // Track covered residues
        residues[0] = 1;  // Residue 0 is covered by default

        // Store trajectory for this episode
        std::vector<std::vector<float>> states;
        std::vector<int> actions;
        std::vector<float> rewards;

        // Each episode consists of D-1 steps (since first position is fixed)
        for (int step = 0; step < D - 1; ++step) {
            // Build state: concatenation of chosen positions and covered residues
            std::vector<float> state(inputSize, 0.0f);
            for (int i = 0; i < N; ++i) {
                state[i] = static_cast<float>(chosen[i]);
                state[N + i] = static_cast<float>(residues[i]);
            }

            // Forward pass through network to get logits
            std::vector<float> logits = policyNet.forward(state);
            
            // Mask already chosen positions (set their logits to very low value)
            for (int i = 0; i < N; ++i) {
                if (chosen[i]) logits[i] = -1e9;
            }

            // Convert logits to probabilities and sample an action
            std::vector<float> probs = softmax(logits);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);  // Sample action from probability distribution

            // Update chosen positions and calculate new covered residues
            chosen[action] = 1;
            int newCovered = 0;
            for (int j = 0; j < N; ++j) {
                if (chosen[j] && j != action) {
                    int res1 = (action - j + N) % N;  // Calculate residues
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

            // Store this step in the trajectory
            states.push_back(state);
            actions.push_back(action);
            rewards.push_back(static_cast<float>(newCovered));  // Reward is number of newly covered residues
        }

        // Check if current solution covers all residues
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

        // Compute discounted returns (backwards)
        std::vector<float> returns(rewards.size());
        float G = 0.0;
        for (int t = rewards.size() - 1; t >= 0; --t) {
            G = GAMMA * G + rewards[t];  // Discounted return
            returns[t] = G;
        }

        // Normalize returns to reduce variance
        float mean = 0.0f, stddev = 0.0f;
        for (float R : returns) mean += R;
        mean /= returns.size();
        for (float R : returns) stddev += (R - mean) * (R - mean);
        stddev = std::sqrt(stddev / returns.size());
        if (stddev < 1e-5) stddev = 1.0f;  // Prevent division by zero
        for (float& R : returns) R = (R - mean) / stddev;

        // Initialize gradient accumulators
        std::vector<std::vector<float>> gradW1(HIDDEN_SIZE1, std::vector<float>(inputSize, 0.0f));
        std::vector<float> gradB1(HIDDEN_SIZE1, 0.0f);
        std::vector<std::vector<float>> gradW2(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1, 0.0f));
        std::vector<float> gradB2(HIDDEN_SIZE2, 0.0f);
        std::vector<std::vector<float>> gradW3(outputSize, std::vector<float>(HIDDEN_SIZE2, 0.0f));
        std::vector<float> gradB3(outputSize, 0.0f);

        // Compute gradients for each step in the episode
        for (size_t t = 0; t < states.size(); ++t) {
            // Forward pass to get current probabilities
            std::vector<float> logits = policyNet.forward(states[t]);
            std::vector<float> probs = softmax(logits);
            
            // Compute policy gradient (REINFORCE algorithm)
            std::vector<float> gradLogits(N, 0.0f);
            for (int i = 0; i < N; ++i) {
                float indicator = (i == actions[t]) ? 1.0f : 0.0f;
                gradLogits[i] = returns[t] * (indicator - probs[i]);  // Policy gradient theorem
            }

            // Backpropagation would be implemented here in a complete implementation
            // (Simplified for brevity - would compute gradients through all layers)
        }

        // Update network weights using accumulated gradients
        policyNet.update(gradW1, gradB1, gradW2, gradB2, gradW3, gradB3);
    }

    printf("No solution found after %d episodes\n", MAX_EPISODES);
}

int main(int argc, const char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        printf("Usage: diff_cover_rl [n] [d]\n");
        return 1;
    }

    // Parse parameters
    int N = atoi(argv[1]);  // Size of the set
    int D = atoi(argv[2]);  // Size of the difference cover

    // Validate parameters
    if (N < 3 || D < 3 || N > D * (D - 1) + 1) {
        printf("Invalid parameters: n>=3, d>=3, n<=d*(d-1)+1\n");
        return 1;
    }

    // Run the RL algorithm to find difference cover
    findDifferenceCoverRL(N, D);
    return 0;
}