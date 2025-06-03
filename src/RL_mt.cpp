/**

**RL implementation for difference cover with multi-threading**

This code is a program that uses artificial intelligence to solve a mathematical puzzle called the "difference cover problem." Think of it like teaching a computer to play a strategic game where it needs to pick the right numbers to win.

**What is the Difference Cover Problem?**

Imagine you have numbers from 0 to N-1 (like 0, 1, 2, 3, 4 if N=5), and you need to pick exactly D of these numbers. The goal is to pick them in such a way that when you look at all the differences between any two picked numbers, those differences cover every possible remainder when divided by N. It's like a number puzzle where your choices need to have a special mathematical property.

**What Input Does It Take?**

The program takes two numbers as command-line arguments:
- N: The total range of numbers to choose from (0 to N-1)
- D: How many numbers you're allowed to pick

For example, if you run the program with N=7 and D=3, you're trying to pick 3 numbers from the set {0, 1, 2, 3, 4, 5, 6}.

**What Output Does It Produce?**

If the program finds a solution, it prints out the specific numbers that form a valid difference cover. If it can't find a solution after trying many times, it reports that no solution was found. The output looks like a list of numbers, such as "0 1 3" which would mean those three numbers solve the puzzle.

**How Does It Work?**

The program uses a technique called "reinforcement learning," which is like teaching a computer through trial and error, similar to how you might learn to play a video game by trying different strategies and getting better over time.

**The AI Brain (Neural Network)**

At the heart of the program is an artificial "brain" called a PolicyNetwork. This brain has three layers of artificial neurons that process information. Think of it like a decision-making system with multiple stages:
- The first layer receives information about the current state of the puzzle
- The middle layers process this information 
- The final layer decides which number to pick next

The brain starts with random decision-making abilities, but it learns and improves over time by remembering what worked well and what didn't.

**The Learning Process**

The program works by playing the "difference cover game" thousands of times. In each game:

1. **State Representation**: The AI looks at the current situation - which numbers have already been picked and which mathematical differences have been covered so far.

2. **Decision Making**: Based on the current state, the neural network calculates probabilities for picking each remaining number. Numbers that seem more promising get higher probabilities.

3. **Action Selection**: The AI randomly selects a number based on these probabilities (so it can explore different strategies, not just pick the most obvious choice every time).

4. **Reward Calculation**: After picking a number, the AI gets a "reward" based on how many new mathematical differences this choice covers. More coverage means a better reward.

5. **Learning**: After completing a full game, the AI analyzes what happened. If the game led to a solution, or if certain moves led to good rewards, the neural network adjusts its internal parameters to make similar decisions more likely in the future.

**Multi-Threading for Speed**

To make the learning process faster, the program runs multiple "worker threads" simultaneously. Think of this as having several AI agents all trying to solve the puzzle at the same time, but they all share the same brain and learn from each other's experiences. This parallel approach speeds up the discovery process significantly.

**The Training Loop**

Each worker thread repeatedly:
- Starts a new puzzle attempt
- Makes a series of number selections using the current AI strategy
- Calculates how good each decision was based on the final outcome
- Updates the shared neural network to be smarter for next time
- Continues until either a solution is found or the maximum number of attempts is reached

**Key Data Transformations**

The program transforms the mathematical problem into a format the AI can understand. The current state of the puzzle gets converted into a list of numbers (called a "state vector") that represents both which numbers have been chosen and which differences have been covered. The neural network processes this numerical representation and outputs probabilities for each possible next move.

The learning process involves calculating "gradients" - mathematical measures of how much each part of the neural network should change to perform better. These gradients are computed by working backwards from the final results to determine what earlier decisions contributed to success or failure.

This approach combines the power of artificial intelligence with parallel computing to tackle a challenging mathematical problem through learned experience rather than traditional algorithmic approaches.

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
constexpr int HIDDEN_SIZE1 = 256;  // Number of neurons in first hidden layer
constexpr int HIDDEN_SIZE2 = 128;  // Number of neurons in second hidden layer
constexpr float LEARNING_RATE = 0.01;  // Learning rate for gradient descent
constexpr float GAMMA = 0.99;  // Discount factor for future rewards
constexpr int MAX_EPISODES = 1000000000;  // Maximum training episodes
constexpr int NUM_THREADS = 10;  // Number of threads to use for parallel training

// Policy Network class implements a neural network for reinforcement learning
class PolicyNetwork {
private:
    int inputSize;  // Size of input layer (2*N for difference cover problem)
    int outputSize;  // Size of output layer (N for difference cover problem)
    std::vector<std::vector<float>> W1, W2, W3;  // Weight matrices for 3 layers
    std::vector<float> b1, b2, b3;  // Bias vectors for 3 layers
    std::mutex networkMutex;  // Mutex for thread-safe network updates

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

        // Initialize first layer weights and biases
        W1.resize(HIDDEN_SIZE1, std::vector<float>(inputSize));
        b1.resize(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] = xavier(inputSize, HIDDEN_SIZE1);
            }
        }

        // Initialize second layer weights and biases
        W2.resize(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1));
        b2.resize(HIDDEN_SIZE2, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);
            }
        }

        // Initialize output layer weights and biases
        W3.resize(outputSize, std::vector<float>(HIDDEN_SIZE2));
        b3.resize(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] = xavier(HIDDEN_SIZE2, outputSize);
            }
        }
    }

    // Forward pass through the network with ReLU activation for hidden layers
    std::vector<float> forward(const std::vector<float>& input) {
        // First hidden layer computation with ReLU activation
        std::vector<float> z1(HIDDEN_SIZE1, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                z1[i] += W1[i][j] * input[j];
            }
            z1[i] += b1[i];
            z1[i] = std::max(0.0f, z1[i]);  // ReLU activation
        }

        // Second hidden layer computation with ReLU activation
        std::vector<float> z2(HIDDEN_SIZE2, 0.0f);
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                z2[i] += W2[i][j] * z1[j];
            }
            z2[i] += b2[i];
            z2[i] = std::max(0.0f, z2[i]);  // ReLU activation
        }

        // Output layer computation (no activation function - returns logits)
        std::vector<float> z3(outputSize, 0.0f);
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                z3[i] += W3[i][j] * z2[j];
            }
            z3[i] += b3[i];
        }
        return z3;
    }

    // Update network weights using gradients (thread-safe operation)
    void update(const std::vector<std::vector<float>>& gradW1,
               const std::vector<float>& gradB1,
               const std::vector<std::vector<float>>& gradW2,
               const std::vector<float>& gradB2,
               const std::vector<std::vector<float>>& gradW3,
               const std::vector<float>& gradB3) {
        std::lock_guard<std::mutex> lock(networkMutex);  // Thread-safe update
        
        // Update first layer weights and biases
        for (int i = 0; i < HIDDEN_SIZE1; ++i) {
            for (int j = 0; j < inputSize; ++j) {
                W1[i][j] -= LEARNING_RATE * gradW1[i][j];
            }
            b1[i] -= LEARNING_RATE * gradB1[i];
        }
        // Update second layer weights and biases
        for (int i = 0; i < HIDDEN_SIZE2; ++i) {
            for (int j = 0; j < HIDDEN_SIZE1; ++j) {
                W2[i][j] -= LEARNING_RATE * gradW2[i][j];
            }
            b2[i] -= LEARNING_RATE * gradB2[i];
        }
        // Update output layer weights and biases
        for (int i = 0; i < outputSize; ++i) {
            for (int j = 0; j < HIDDEN_SIZE2; ++j) {
                W3[i][j] -= LEARNING_RATE * gradW3[i][j];
            }
            b3[i] -= LEARNING_RATE * gradB3[i];
        }
    }
};

// Softmax function converts logits to probability distribution
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

// Worker thread function for parallel training
void workerThread(PolicyNetwork& policyNet, int N, int D, 
                 std::atomic<int>& episodeCounter, std::atomic<bool>& solutionFound,
                 std::mutex& outputMutex) {
    std::mt19937 gen(std::random_device{}());  // Random number generator
    
    // Main training loop for each thread
    while (!solutionFound && episodeCounter < MAX_EPISODES) {
        int episode = episodeCounter++;
        if (episode >= MAX_EPISODES) break;

        // Initialize problem state for difference cover
        std::vector<int> chosen(N, 0);  // Track chosen elements
        chosen[0] = 1;  // Start with first element chosen
        std::vector<int> residues(N, 0);  // Track covered residues
        residues[0] = 1;  // 0 is always covered

        // Store episode data for training
        std::vector<std::vector<float>> states;
        std::vector<int> actions;
        std::vector<float> rewards;

        // Generate episode by interacting with environment
        for (int step = 0; step < D - 1; ++step) {
            // Create state representation: concatenation of chosen and residues
            std::vector<float> state(2 * N, 0.0f);
            for (int i = 0; i < N; ++i) {
                state[i] = static_cast<float>(chosen[i]);
                state[N + i] = static_cast<float>(residues[i]);
            }

            // Get action probabilities from policy network
            std::vector<float> logits = policyNet.forward(state);
            
            // Mask already chosen elements by setting their logits to very low value
            for (int i = 0; i < N; ++i) {
                if (chosen[i]) logits[i] = -1e9;
            }

            // Sample action from probability distribution
            std::vector<float> probs = softmax(logits);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);

            // Update environment state based on chosen action
            chosen[action] = 1;
            int newCovered = 0;
            for (int j = 0; j < N; ++j) {
                if (chosen[j] && j != action) {
                    // Calculate new residues covered by this action
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

            // Store experience for training
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

        // If solution found, print it and set flag
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

        // Calculate discounted returns for each step
        std::vector<float> returns(rewards.size());
        float G = 0.0;
        for (int t = rewards.size() - 1; t >= 0; --t) {
            G = GAMMA * G + rewards[t];  // Discounted return
            returns[t] = G;
        }

        // Normalize returns for more stable training
        float mean = 0.0f, stddev = 0.0f;
        for (float R : returns) mean += R;
        mean /= returns.size();
        for (float R : returns) stddev += (R - mean) * (R - mean);
        stddev = std::sqrt(stddev / returns.size());
        if (stddev < 1e-5) stddev = 1.0f;  // Avoid division by zero
        for (float& R : returns) R = (R - mean) / stddev;  // Standardize returns

        // Initialize gradients for all weights and biases
        std::vector<std::vector<float>> gradW1(HIDDEN_SIZE1, std::vector<float>(2 * N, 0.0f));
        std::vector<float> gradB1(HIDDEN_SIZE1, 0.0f);
        std::vector<std::vector<float>> gradW2(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1, 0.0f));
        std::vector<float> gradB2(HIDDEN_SIZE2, 0.0f);
        std::vector<std::vector<float>> gradW3(N, std::vector<float>(HIDDEN_SIZE2, 0.0f));
        std::vector<float> gradB3(N, 0.0f);

        // Calculate gradients for each time step (policy gradient theorem)
        for (size_t t = 0; t < states.size(); ++t) {
            std::vector<float> logits = policyNet.forward(states[t]);
            std::vector<float> probs = softmax(logits);
            
            std::vector<float> gradLogits(N, 0.0f);
            for (int i = 0; i < N; ++i) {
                float indicator = (i == actions[t]) ? 1.0f : 0.0f;
                gradLogits[i] = returns[t] * (indicator - probs[i]);  // Policy gradient
            }
        }

        // Update network weights with calculated gradients
        policyNet.update(gradW1, gradB1, gradW2, gradB2, gradW3, gradB3);
    }
}

// Main function to find difference cover using reinforcement learning
void findDifferenceCoverRL(int N, int D) {
    const int inputSize = 2 * N;  // Input size is twice N (chosen + residues)
    PolicyNetwork policyNet(inputSize, N);  // Initialize policy network
    
    // Shared variables for multi-threading
    std::atomic<int> episodeCounter(0);  // Tracks total episodes across threads
    std::atomic<bool> solutionFound(false);  // Flag when solution is found
    std::mutex outputMutex;  // Mutex for synchronized output

    // Create and launch worker threads
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(workerThread, std::ref(policyNet), N, D, 
                           std::ref(episodeCounter), std::ref(solutionFound),
                           std::ref(outputMutex));
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Print result if no solution was found
    if (!solutionFound) {
        printf("No solution found after %d episodes\n", MAX_EPISODES);
    }
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

    // Run the difference cover search
    findDifferenceCoverRL(N, D);
    return 0;
}