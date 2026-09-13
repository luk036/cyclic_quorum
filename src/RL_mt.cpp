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
constexpr float GAMMA = 0.98;  // Discount factor for future rewards
constexpr float ENTROPY_BETA = 0.01f;  // Entropy bonus weight to preserve exploration
constexpr int MAX_N = 1000000;  // Upper bound on N (keeps int size arithmetic safe)
constexpr int MAX_EPISODES = 1000000000;  // Maximum training episodes
constexpr int NUM_THREADS = 10;  // Number of threads to use for parallel training

// Intermediate activations captured during a forward pass, needed for backpropagation
struct ForwardResult {
    std::vector<float> z1_pre;  // Pre-activation of first hidden layer
    std::vector<float> z1;      // Post-ReLU activation of first hidden layer
    std::vector<float> z2_pre;  // Pre-activation of second hidden layer
    std::vector<float> z2;      // Post-ReLU activation of second hidden layer
    std::vector<float> z3;      // Output logits
};

// Policy Network class implements a neural network for reinforcement learning
class PolicyNetwork {
private:
    int inputSize;  // Size of input layer (2*N for difference cover problem)
    int outputSize;  // Size of output layer (N for difference cover problem)
    std::vector<std::vector<float>> W1, W2, W3;  // Weight matrices for 3 layers
    std::vector<float> b1, b2, b3;  // Bias vectors for 3 layers

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
        for (int row = 0; row < HIDDEN_SIZE1; ++row) {
            for (int col = 0; col < inputSize; ++col) {
                W1[row][col] = xavier(inputSize, HIDDEN_SIZE1);
            }
        }

        // Initialize second layer weights and biases
        W2.resize(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1));
        b2.resize(HIDDEN_SIZE2, 0.0f);
        for (int row = 0; row < HIDDEN_SIZE2; ++row) {
            for (int col = 0; col < HIDDEN_SIZE1; ++col) {
                W2[row][col] = xavier(HIDDEN_SIZE1, HIDDEN_SIZE2);
            }
        }

        // Initialize output layer weights and biases
        W3.resize(outputSize, std::vector<float>(HIDDEN_SIZE2));
        b3.resize(outputSize, 0.0f);
        for (int row = 0; row < outputSize; ++row) {
            for (int col = 0; col < HIDDEN_SIZE2; ++col) {
                W3[row][col] = xavier(HIDDEN_SIZE2, outputSize);
            }
        }
    }

    // Forward pass through the network with ReLU activation for hidden layers.
    // Fills a caller-supplied buffer so it can be reused across episodes.
    void forward(const std::vector<float>& input, ForwardResult& fr) const {
        // First hidden layer computation with ReLU activation
        fr.z1_pre.resize(HIDDEN_SIZE1);
        fr.z1.resize(HIDDEN_SIZE1);
        for (int row = 0; row < HIDDEN_SIZE1; ++row) {
            float sum = b1[row];
            for (int col = 0; col < inputSize; ++col) {
                sum += W1[row][col] * input[col];
            }
            fr.z1_pre[row] = sum;
            fr.z1[row] = std::max(0.0f, sum);  // ReLU activation
        }

        // Second hidden layer computation with ReLU activation
        fr.z2_pre.resize(HIDDEN_SIZE2);
        fr.z2.resize(HIDDEN_SIZE2);
        for (int row = 0; row < HIDDEN_SIZE2; ++row) {
            float sum = b2[row];
            for (int col = 0; col < HIDDEN_SIZE1; ++col) {
                sum += W2[row][col] * fr.z1[col];
            }
            fr.z2_pre[row] = sum;
            fr.z2[row] = std::max(0.0f, sum);  // ReLU activation
        }

        // Output layer computation (no activation function - returns logits)
        fr.z3.resize(outputSize);
        for (int row = 0; row < outputSize; ++row) {
            float sum = b3[row];
            for (int col = 0; col < HIDDEN_SIZE2; ++col) {
                sum += W3[row][col] * fr.z2[col];
            }
            fr.z3[row] = sum;
        }
    }

    // Backpropagates the REINFORCE policy gradient for a single step and
    // accumulates the parameter gradients. We maximise the expected return plus
    // an entropy bonus, so
    //   dL/dlogit_i = return * (probs_i - indicator_i)
    //               + ENTROPY_BETA * probs_i * (log probs_i + entropy),
    // where L is the loss that update() descends.
    void backward(const ForwardResult& fr, const std::vector<float>& input,
                  const std::vector<float>& probs, int action, float return_val,
                  std::vector<std::vector<float>>& gradW1, std::vector<float>& gradB1,
                  std::vector<std::vector<float>>& gradW2, std::vector<float>& gradB2,
                  std::vector<std::vector<float>>& gradW3, std::vector<float>& gradB3) const {
        float entropy = 0.0f;
        for (int idx = 0; idx < outputSize; ++idx) {
            entropy -= probs[idx] * std::log(probs[idx] + 1e-10f);
        }

        std::vector<float> gradLogits(outputSize, 0.0f);
        for (int idx = 0; idx < outputSize; ++idx) {
            float indicator = (idx == action) ? 1.0f : 0.0f;
            gradLogits[idx] = return_val * (probs[idx] - indicator)
                            + ENTROPY_BETA * probs[idx] * (std::log(probs[idx] + 1e-10f) + entropy);
        }

        for (int row = 0; row < outputSize; ++row) {
            for (int col = 0; col < HIDDEN_SIZE2; ++col) {
                gradW3[row][col] += gradLogits[row] * fr.z2[col];
            }
            gradB3[row] += gradLogits[row];
        }

        std::vector<float> dz2(HIDDEN_SIZE2, 0.0f);
        for (int col = 0; col < HIDDEN_SIZE2; ++col) {
            float sum = 0.0f;
            for (int row = 0; row < outputSize; ++row) {
                sum += W3[row][col] * gradLogits[row];
            }
            dz2[col] = fr.z2_pre[col] > 0.0f ? sum : 0.0f;  // ReLU derivative
        }
        for (int row = 0; row < HIDDEN_SIZE2; ++row) {
            for (int col = 0; col < HIDDEN_SIZE1; ++col) {
                gradW2[row][col] += dz2[row] * fr.z1[col];
            }
            gradB2[row] += dz2[row];
        }

        std::vector<float> dz1(HIDDEN_SIZE1, 0.0f);
        for (int col = 0; col < HIDDEN_SIZE1; ++col) {
            float sum = 0.0f;
            for (int row = 0; row < HIDDEN_SIZE2; ++row) {
                sum += W2[row][col] * dz2[row];
            }
            dz1[col] = fr.z1_pre[col] > 0.0f ? sum : 0.0f;  // ReLU derivative
        }
        for (int row = 0; row < HIDDEN_SIZE1; ++row) {
            for (int col = 0; col < inputSize; ++col) {
                gradW1[row][col] += dz1[row] * input[col];
            }
            gradB1[row] += dz1[row];
        }
    }

    // Update network weights using gradients. Lock-free (Hogwild-style async SGD).
    // This is an intentional unsynchronised read-modify-write: concurrent updates
    // and forward() reads form a C++ data race (formally UB). It relies on
    // naturally-aligned 4-byte float accesses being tear-free on the target
    // (x86-64/AArch64); lost updates are the intended noise, and it scales far
    // better than serialising every update behind a mutex.
    void update(const std::vector<std::vector<float>>& gradW1,
               const std::vector<float>& gradB1,
               const std::vector<std::vector<float>>& gradW2,
               const std::vector<float>& gradB2,
               const std::vector<std::vector<float>>& gradW3,
               const std::vector<float>& gradB3) {
        // Update first layer weights and biases
        for (int row = 0; row < HIDDEN_SIZE1; ++row) {
            for (int col = 0; col < inputSize; ++col) {
                W1[row][col] -= LEARNING_RATE * gradW1[row][col];
            }
            b1[row] -= LEARNING_RATE * gradB1[row];
        }
        // Update second layer weights and biases
        for (int row = 0; row < HIDDEN_SIZE2; ++row) {
            for (int col = 0; col < HIDDEN_SIZE1; ++col) {
                W2[row][col] -= LEARNING_RATE * gradW2[row][col];
            }
            b2[row] -= LEARNING_RATE * gradB2[row];
        }
        // Update output layer weights and biases
        for (int row = 0; row < outputSize; ++row) {
            for (int col = 0; col < HIDDEN_SIZE2; ++col) {
                W3[row][col] -= LEARNING_RATE * gradW3[row][col];
            }
            b3[row] -= LEARNING_RATE * gradB3[row];
        }
    }
};

// Softmax function converts logits to probability distribution.
// Writes into the caller-supplied buffer so it can be reused across episodes.
void softmax(const std::vector<float>& logits, std::vector<float>& probs) {
    float maxLogit = *std::max_element(logits.begin(), logits.end());  // For numerical stability
    float sumExp = 0.0f;
    for (size_t idx = 0; idx < logits.size(); ++idx) {
        probs[idx] = std::exp(logits[idx] - maxLogit);
        sumExp += probs[idx];
    }
    for (size_t idx = 0; idx < probs.size(); ++idx) {
        probs[idx] /= sumExp;  // Normalize to get probabilities
    }
}

// Worker thread function for parallel training
void workerThread(PolicyNetwork& policyNet, int N, int D,
                 std::atomic<int>& episodeCounter, std::atomic<bool>& solutionFound,
                 std::mutex& outputMutex) {
    std::mt19937 gen(std::random_device{}());  // Random number generator

    const int T = D - 1;
    const int IN = 2 * N;

    // Per-thread buffers reused across episodes to avoid per-episode allocation
    std::vector<int> chosen(N);    // Track chosen elements
    std::vector<int> residues(N);  // Track covered residues
    std::vector<std::vector<float>> states(T, std::vector<float>(IN));
    std::vector<std::vector<float>> probsList(T, std::vector<float>(N));
    std::vector<ForwardResult> fwdList(T);
    std::vector<int> actions(T);
    std::vector<float> rewards(T);
    std::vector<float> returns(T);

    std::vector<std::vector<float>> gradW1(HIDDEN_SIZE1, std::vector<float>(IN, 0.0f));
    std::vector<float> gradB1(HIDDEN_SIZE1, 0.0f);
    std::vector<std::vector<float>> gradW2(HIDDEN_SIZE2, std::vector<float>(HIDDEN_SIZE1, 0.0f));
    std::vector<float> gradB2(HIDDEN_SIZE2, 0.0f);
    std::vector<std::vector<float>> gradW3(N, std::vector<float>(HIDDEN_SIZE2, 0.0f));
    std::vector<float> gradB3(N, 0.0f);

    // Main training loop for each thread
    while (!solutionFound && episodeCounter < MAX_EPISODES) {
        int episode = episodeCounter++;
        if (episode >= MAX_EPISODES) break;

        // Initialize problem state for difference cover
        std::fill(chosen.begin(), chosen.end(), 0);
        chosen[0] = 1;  // Start with first element chosen
        std::fill(residues.begin(), residues.end(), 0);
        residues[0] = 1;  // 0 is always covered

        // Generate episode by interacting with environment
        for (int step = 0; step < T; ++step) {
            // Create state representation: concatenation of chosen and residues
            std::vector<float>& state = states[step];
            for (int idx = 0; idx < N; ++idx) {
                state[idx] = static_cast<float>(chosen[idx]);
                state[N + idx] = static_cast<float>(residues[idx]);
            }

            // Get action probabilities from policy network
            ForwardResult& fr = fwdList[step];
            policyNet.forward(state, fr);

            // Mask already chosen elements by setting their logits to very low value
            for (int idx = 0; idx < N; ++idx) {
                if (chosen[idx]) fr.z3[idx] = -1e9f;
            }

            // Sample action from probability distribution
            std::vector<float>& probs = probsList[step];
            softmax(fr.z3, probs);
            std::discrete_distribution<int> dist(probs.begin(), probs.end());
            int action = dist(gen);

            // Update environment state based on chosen action
            chosen[action] = 1;
            int newCovered = 0;
            for (int idx = 0; idx < N; ++idx) {
                if (chosen[idx] && idx != action) {
                    // Calculate new residues covered by this action
                    int res1 = (action - idx + N) % N;
                    int res2 = (idx - action + N) % N;
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
            actions[step] = action;
            rewards[step] = static_cast<float>(newCovered);  // Reward is number of newly covered residues
        }

        // Check if current solution covers all residues
        bool isSolution = true;
        for (int residue : residues) {
            if (!residue) {
                isSolution = false;
                break;
            }
        }

        // If solution found, print it and set flag
        if (isSolution) {
            std::lock_guard<std::mutex> lock(outputMutex);
            solutionFound = true;
            printf("\nSolution found in episode %d:\n", episode);
            for (int idx = 0; idx < N; ++idx) {
                if (chosen[idx]) printf("%d ", idx);
            }
            printf("\n");
            return;
        }

        // Calculate discounted returns for each step
        float G = 0.0;
        for (int t = T - 1; t >= 0; --t) {
            G = GAMMA * G + rewards[t];  // Discounted return
            returns[t] = G;
        }

        // Normalize returns for more stable training
        float mean = 0.0f, stddev = 0.0f;
        for (int t = 0; t < T; ++t) mean += returns[t];
        mean /= T;
        for (int t = 0; t < T; ++t) stddev += (returns[t] - mean) * (returns[t] - mean);
        stddev = std::sqrt(stddev / T);
        if (stddev < 1e-5) stddev = 1.0f;  // Avoid division by zero
        for (int t = 0; t < T; ++t) returns[t] = (returns[t] - mean) / stddev;  // Standardize returns

        for (auto& row : gradW1) std::fill(row.begin(), row.end(), 0.0f);
        std::fill(gradB1.begin(), gradB1.end(), 0.0f);
        for (auto& row : gradW2) std::fill(row.begin(), row.end(), 0.0f);
        std::fill(gradB2.begin(), gradB2.end(), 0.0f);
        for (auto& row : gradW3) std::fill(row.begin(), row.end(), 0.0f);
        std::fill(gradB3.begin(), gradB3.end(), 0.0f);

        // Calculate gradients for each time step (policy gradient theorem)
        for (int t = 0; t < T; ++t) {
            policyNet.backward(fwdList[t], states[t], probsList[t], actions[t], returns[t],
                               gradW1, gradB1, gradW2, gradB2, gradW3, gradB3);
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
    for (int idx = 0; idx < NUM_THREADS; ++idx) {
        threads.emplace_back(workerThread, std::ref(policyNet), N, D,
                           std::ref(episodeCounter), std::ref(solutionFound),
                           std::ref(outputMutex));
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
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

    // Validate parameters. Bound D and N before the product so allocation-size
    // arithmetic cannot overflow int, and the product itself cannot overflow.
    if (N < 3 || D < 3 || D > N || N > MAX_N ||
        static_cast<long long>(N) > static_cast<long long>(D) * (D - 1) + 1) {
        printf("Invalid parameters: n>=3, d>=3, n<=d*(d-1)+1\n");
        return 1;
    }

    // Run the difference cover search
    findDifferenceCoverRL(N, D);
    return 0;
}