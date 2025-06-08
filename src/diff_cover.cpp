/**

**diff_cover.cpp**

This program is designed to find special mathematical arrangements called "difference covers" or "difference sets." Think of it as a puzzle solver that looks for specific patterns in numbers.

**Purpose of the Code**

The program searches for sets of numbers that have a unique mathematical property: when you calculate all possible differences between pairs of numbers in the set, those differences cover a wide range of values without too much repetition. This is similar to finding a special combination of numbers where their spacing creates an optimal pattern. These mathematical structures have applications in areas like coding theory, cryptography, and signal processing.

**Input and Output**

The program takes two command-line arguments: N (the total range of numbers to work with) and D (how many numbers to include in each set). For example, if you run the program with N=15 and D=5, it will look for sets of 5 numbers within the range 0 to 14. The program validates that N is at least 3, D is at least 3, and that N doesn't exceed D*(D-1)+1, which ensures the mathematical problem has valid solutions.

When the program finds valid arrangements, it prints them to the screen as rows of numbers. Each row represents one valid difference cover that meets the mathematical criteria. The program also displays progress information, including how many worker threads it's using and a countdown showing how much work remains.

**How It Achieves Its Purpose**

The program uses a sophisticated recursive search algorithm combined with parallel processing. The main logic is contained in the DcGenerator class, which systematically explores all possible combinations of numbers. It starts with a base configuration and then tries different values for each position, checking at each step whether the current partial solution could lead to a valid complete solution.

The algorithm uses several optimization techniques to avoid wasting time on impossible solutions. It maintains arrays to track which differences have already been seen and counts how many unique differences exist. Before exploring deeper into a potential solution, it checks whether there are enough unique differences to possibly reach a valid final answer. If not, it abandons that path early and tries a different approach.

**Important Logic Flows and Data Transformations**

The program follows a generate-and-test approach with intelligent pruning. It builds potential solutions incrementally, adding one number at a time to the current set. For each new number added, it calculates all the differences between this number and the previously selected numbers, updating its internal tracking of which differences have been seen.

The core algorithm uses backtracking, which means when it reaches a dead end (a partial solution that cannot lead to a valid complete solution), it backs up and tries different choices. The step_forward and step_backward functions manage this process by updating and reverting the difference counts as the algorithm explores and backtracks.

To speed up the computation, the program uses parallel processing through a thread pool. It divides the work by having different threads start their searches from different initial values. This allows multiple CPU cores to work on the problem simultaneously, significantly reducing the total time needed to find all solutions. The program coordinates these parallel workers and displays progress as they complete their assigned portions of the search space.
*/

/*
 * Copyright (c) 2019 Joe Sawada
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>  // for memset, memcpy
#include <future>   // for future
#include <vector>

#include "ThreadPool.h"

// Maximum constants for array sizes
static constexpr int MAX_C = 128;  // Maximum number of distinct differences
static constexpr int MAX_D = 20;   // Maximum density for the problem

class DcGenerator {
  private:
    const int N;  // Total number of elements
    const int D;  // Density of the problem
    const int ND; // N - D
    const int D1; // D - 1
    const int N2; // N / 2
    const int N1; // N2 - D*D1/2 (threshold for valid configurations)

    int a[MAX_D];  // Array to store current configuration
    int b[MAX_D];  // Array to store auxiliary information
    int differences[MAX_C];  // Array to track differences between elements
    // int count;  // Commented out count variable

  public:
    // Constructor initializes the generator with parameters n, d, and starting value j
    DcGenerator(int n, int d, int j)
        : N(n), D(d), ND(N - D), N2(N / 2), D1(D - 1), N1(N2 - D * D1 / 2) {
        // Initialize arrays to zero
        std::memset(a, 0, sizeof(a));
        std::memset(b, 0, sizeof(b));
        std::memset(differences, 0, sizeof(differences));

        a[D] = N;  // Sentinel value for printing
        a[0] = 0;  // Base value for computations
        a[1] = j;  // Starting value
        b[1] = 1;   // Initial auxiliary value
        differences[0] = 1;  // Difference of 0 is always present
    }

    // Updates the difference counts when moving forward in the generation
    inline void step_forward(int t, int &count) {
        const int at = a[t];
        for (int j = 0; j < t; ++j) {
            const int aj = a[j];
            const int p_diff = at - aj;  // Positive difference
            const int n_diff = N - p_diff;  // Negative difference (modular)
            const int diff = p_diff < n_diff ? p_diff : n_diff;  // Minimum difference

            // Note that p_diff may be equal to n_diff
            if (differences[diff]++ == 0) {
                ++count;  // Increment count if this is a new unique difference
            }
        }
    }

    // Reverts the difference counts when backtracking
    inline void step_backward(int t) {
        const int at = a[t];
        for (int j = 0; j < t; ++j) {
            const int aj = a[j];
            const int p_diff = at - aj;
            const int n_diff = N - p_diff;
            const int diff = p_diff < n_diff ? p_diff : n_diff;
            --differences[diff];  // Decrement the difference count
            // Note that p_diff may be equal to n_diff
            // if (--differences[diff] == 0) {
            //     --count;
            // }
        }
    }

    // Prints the valid configuration when found
    void PrintD(int p, int count) {
        /* Determine minimum position for next bit */
        const int Dp = D % p;
        const int next = (D / p) * a[p] + a[Dp];
        if (next < N) return;  // Skip if not a complete configuration

        /* Determine last bit */
        int min = 1;
        if (next == N) {
            min = Dp != 0 ? b[Dp] + 1 : b[p];
        }

        if (min != 1) return;  // Skip if not minimal

        // Check if we have enough unique differences
        step_forward(D1, count);
        if (count >= N2) {
            printf("\n");
            for (int i = 1; i <= D; ++i) {
                printf("%3d ", a[i]);  // Print the valid configuration
            }
            printf("\n");
            fflush(stdout);
        }
        step_backward(D1);
    }

    // Recursive generation function for the configurations
    void GenD(int t, int p, int count) {
        if (t >= D1) {  // Base case: reached depth D-1
            PrintD(p, count);
            return;
        }

        const int t_1 = t + 1;
        step_forward(t, count);  // Update differences for current position

        // Continue if we still have enough unique differences
        if (count >= N1 + t * t_1 / 2) {
            int tail = ND + t_1;
            const int max = a[t_1 - p] + a[p];

            // Try the maximum possible value first
            if (max <= tail) {
                a[t_1] = max;
                b[t_1] = b[t_1 - p];
                GenD(t_1, p, count);
                tail = max - 1;
            }

            // Try all other possible values in descending order
            for (int j = tail; j >= a[t] + 1; --j) {
                a[t_1] = j;
                b[t_1] = 1;
                GenD(t_1, t_1, count);
            }
        }
        step_backward(t);  // Backtrack the difference counts
    }

    // Initial generation function that starts the process
    void Gen11() {
        int count = 0;
        step_forward(1, count);  // Initialize with first element
        int tail = ND + 2;
        const int max = a[1] + a[1];

        // Try the maximum possible value first
        if (max <= tail) {
            a[2] = max;
            b[2] = b[1];
            GenD(2, 1, count);
            tail = max - 1;
        }

        // Try all other possible values in descending order
        for (int j = tail; j >= a[1] + 1; --j) {
            a[2] = j;
            b[2] = 1;
            GenD(2, 2, count);
        }
        // step_backward(1);
    }

    // Display usage information
    static void usage() { printf("Usage: necklace [n] [d] (n>=3, d>=3, n<=d*(d-1)+1)\n"); }
};

// Initialize parallel computation with thread pool
void InitParallel(int N, int D) {
    const unsigned num_workers = std::thread::hardware_concurrency();
    ThreadPool pool(num_workers);
    printf("Number of workers: %u\n", num_workers / 2);

    std::vector<std::future<void>> results;
    results.reserve((N + 1) / 2 - (N - 1) / D);  // Pre-allocate space

    const int start = (N + 1) / 2;
    const int end = (N - 1) / D + 1;

    // Enqueue work items for each starting value in parallel
    for (int j = start; j >= end; --j) {
        results.emplace_back(pool.enqueue([N, D, j]() {
            DcGenerator generator(N, D, j);
            generator.Gen11();
        }));
    }
    int countdown = start - end;
    for (auto &result : results) {
        printf("%3d\r", countdown--);  // Display progress
        fflush(stdout);
        result.get();  // Wait for completion
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        DcGenerator::usage();
        return 1;
    }

    int N = atoi(argv[1]);
    int D = atoi(argv[2]);

    // Validate input parameters
    if (N < 3 || D < 3 || N > D * (D - 1) + 1) {
        DcGenerator::usage();
        return 1;
    }

    InitParallel(N, D);  // Start parallel computation
    printf("Finished successfully\n");
    return 0;
}