/*
**diff_cover3.cpp**

This code is a mathematical program that searches for special number sequences called "difference covers." A difference cover is a set of numbers where the differences between any two numbers in the set can represent all possible values within a certain range. Think of it like finding a minimal set of positions on a number line such that the distances between these positions cover all the gaps you might need.

**Purpose and Input**

The program takes two command-line arguments: N (the range size) and D (the size of the difference cover set). For example, if you run the program with N=15 and D=5, it will search for sets of 5 numbers within the range 0 to 14 that form valid difference covers. The program has built-in constraints requiring both N and D to be at least 3, and N cannot exceed D*(D-1)+1.

**Output**

When the program finds valid difference covers, it prints them as sequences of numbers. Each valid sequence is displayed on a separate line, showing the D numbers that make up the difference cover. The program also displays progress information, including the number of worker threads being used and a countdown showing how many search tasks remain.

**How It Works**

The program uses a sophisticated backtracking algorithm implemented in the `DcGenerator` class. This algorithm systematically builds potential difference covers one number at a time, checking at each step whether the partial sequence could lead to a valid complete difference cover. The core idea is to maintain arrays that track which numbers have been used (`q` array) and which difference values have been covered (`differences` array).

The algorithm starts with a fixed first number and then tries different possibilities for subsequent positions. For each potential number it might add to the sequence, it calculates what new differences would be created and checks if adding this number brings the sequence closer to covering all required differences. If a partial sequence looks promising (meaning it has covered enough differences for its current length), the algorithm continues building on it. If not, it backtracks and tries a different number.

**Key Logic Flow**

The program employs parallel processing to speed up the search. It divides the work by trying different starting values simultaneously across multiple threads. Each thread runs its own instance of the search algorithm (`BraceFD11` method), which recursively explores possible sequences using the `BraceFD` method.

Two critical functions manage the search state: `step_forward` adds a new number to the current sequence and updates the count of covered differences, while `step_backward` removes a number and undoes those updates. This allows the algorithm to efficiently explore and backtrack through the search space.

The program also includes symmetry-breaking optimizations through the `CheckRev` function, which helps avoid generating equivalent sequences that are just mirror images of each other. This significantly reduces the search space without missing any fundamentally different solutions.

When a complete sequence of D numbers is built, the `PrintD` function performs final validation checks to ensure it forms a proper difference cover before printing it as output. The program continues until all possible valid difference covers for the given parameters have been found and displayed.
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

static constexpr int MAX_N = 256;
static constexpr int MAX_C = 128;
static constexpr int MAX_D = 20;

class DcGenerator {
  private:
    const int N;
    const int D;
    const int ND;
    const int D1;
    const int N2;
    const int N1;

    int a[MAX_D];
    int q[MAX_N];
    // int s[MAX_N];

    int differences[MAX_C];
    // int count;

  public:
    DcGenerator(int n, int d, int j)
        : N(n), D(d), ND(N - D), N2(N / 2), D1(D - 1), N1(N2 - D * D1 / 2) {
        // Initialize arrays to zero
        std::memset(a, 0, sizeof(a));
        std::memset(q, 0, sizeof(q));
        // std::memset(s, 0, sizeof(s));
        std::memset(differences, 0, sizeof(differences));

        a[D] = N;  // for generating bracelets
        a[0] = 0;  // for computing difference cover
        // s[N] = D;

        a[1] = j;
        q[j] = 1;
        differences[0] = 1;
    }

    int CheckRev(int t_1) {
        for (int idx = a[1]; idx <= t_1 / 2; ++idx) {
            if (q[idx] < q[t_1 - idx]) return 1;
            if (q[idx] > q[t_1 - idx]) return -1;
        }
        return 0;
    }

    inline void step_forward(int t, int &count) {
        const int at = a[t];
        for (int idx = 0; idx < t; ++idx) {
            const int aj = a[idx];
            const int p_diff = at - aj;
            const int n_diff = N - p_diff;
            const int diff = p_diff < n_diff ? p_diff : n_diff;

            // Note that p_diff may be equal to n_diff
            if (differences[diff]++ == 0) {
                ++count;
            }
        }
    }

    inline void step_backward(int t) {
        const int at = a[t];
        for (int idx = 0; idx < t; ++idx) {
            const int aj = a[idx];
            const int p_diff = at - aj;
            const int n_diff = N - p_diff;
            const int diff = p_diff < n_diff ? p_diff : n_diff;
            --differences[diff];
            // Note that p_diff may be equal to n_diff
            // if (--differences[diff] == 0) {
            //     --count;
            // }
        }
    }

    void PrintD(int p, int count) {
        /* Determine minimum position for next bit */
        const int Dp = D % p;
        const int next = (D / p) * a[p] + a[Dp];
        if (next < N) return;

        /* Determine last bit */
        int min = 1;
        if (next == N) {
            min = Dp != 0 ? q[a[Dp]] + 1 : q[a[p]];
        }
        if (min != 1) return;

        // if (min == 1) {
        step_forward(D1, count);
        if (count >= N2) {
            printf("\n");
            for (int idx = 1; idx <= D; ++idx) {
                printf("%3d ", a[idx]);
            }
            printf("\n");
            fflush(stdout);
        }
        step_backward(D1);
        // }
    }

    void BraceFD(int t, int p, int r1, int count) {
        if (t >= D1) {
            PrintD(p, count);
            return;
        }

        const int at = a[t];
        // if (at > (N - r1 + 1) / 2 + r1 - 1) {
        //     int e = N - at + r1;
        //     if (q[at] > q[e]) RS = false;
        //     else if (q[at] < q[e]) RS = true;
        //     else if (at - a[t - 1] > a[s[e] + 1] - a[s[e]]) {
        //         RS = true;
        //     }
        // }

        const int t_1 = t + 1;
        step_forward(t, count);

        if (count >= N1 + t * t_1 / 2) {
            int tail = ND + t_1;
            const int max = a[t_1 - p] + a[p];

            if (max <= tail) {
                int r2 = r1;
                // bool RS2 = RS;
                a[t_1] = max;
                q[max] = q[a[t_1 - p]];
                // s[max] = t_1;
                if (a[1] == max - at) {
                    int rev = CheckRev(max);
                    if (rev == 0) {
                        r2 = max;
                        // RS2 = false;
                    }
                    if (rev != -1) {
                        BraceFD(t_1, p, r2, count);
                    }
                } else {
                    BraceFD(t_1, p, r1, count);
                }
                q[max] = 0;
                // s[max] = 0;
                tail = max - 1;
            }
            for (int idx = tail; idx >= at + 1; --idx) {
                a[t_1] = idx;
                // s[idx] = t_1;
                q[idx] = 1;
                BraceFD(t_1, t_1, r1, count);
                q[idx] = 0;
                // s[idx] = 0;
            }
        }
        step_backward(t);
    }

    void BraceFD11() {
        const int a1 = a[1];
        const int r1 = a1;
        // const bool RS = false;

        // if (0 > (N - r) / 2 - 1) {
        //     printf("Boom!!!\n");
        //     exit(1);
        // }

        int count = 0;
        step_forward(1, count);
        int tail = ND + 2;
        const int max = a1 + a1;
        if (max <= tail) {
            int r2 = r1;
            // bool RS2 = RS;
            a[2] = max;
            q[max] = q[a1];
            // s[max] = 2;
            if (a1 == max - a1) {
                // if (CheckRev(max) != 0) {
                //     printf("Boom2!!!\n");
                //     exit(2);
                // }
                r2 = max;
                // RS2 = false;
                BraceFD(2, 1, r2, count);
            } else {
                BraceFD(2, 1, r1, count);
            }
            q[max] = 0;
            // s[max] = 0;
            tail = max - 1;
        }

        for (int idx = tail; idx >= a1 + 1; --idx) {
            a[2] = idx;
            // s[idx] = 2;
            q[idx] = 1;
            BraceFD(2, 2, r1, count);
            q[idx] = 0;
            // s[idx] = 0;
        }
        // step_backward(1);
    }

    static void usage() { printf("Usage: necklace [n] [d] (n>=3, d>=3, n<=d*(d-1)+1)\n"); }
};

void InitParallel(int N, int D) {
    const unsigned num_workers = std::thread::hardware_concurrency();
    ThreadPool pool(num_workers);
    printf("Number of workers: %u\n", num_workers / 2);

    std::vector<std::future<void>> results;
    results.reserve((N + 1) / 2 - (N - 1) / D);  // Pre-allocate space

    const int start = (N + 1) / 2;
    const int end = (N - 1) / D + 1;

    for (int idx = start; idx >= end; --idx) {
        results.emplace_back(pool.enqueue([N, D, idx]() {
            DcGenerator generator(N, D, idx);
            generator.BraceFD11();
        }));
    }
    int countdown = start - end;
    for (auto &result : results) {
        printf("%3d\r", countdown--);
        fflush(stdout);
        result.get();
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

    if (N < 3 || D < 3 || N > D * (D - 1) + 1) {
        DcGenerator::usage();
        return 1;
    }

    InitParallel(N, D);
    printf("Finished successfully\n");
    return 0;
}
