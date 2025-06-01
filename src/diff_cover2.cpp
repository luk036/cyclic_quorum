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
    // int b[MAX_D];
    int differences[MAX_C];
    // int count;

  public:
    DcGenerator(int n, int d, int j)
        : N(n), D(d), ND(N - D), N2(N / 2), D1(D - 1), N1(N2 - D * D1 / 2) {
        // Initialize arrays to zero
        std::memset(a, 0, sizeof(a));
        // std::memset(b, 0, sizeof(b));
        std::memset(differences, 0, sizeof(differences));

        a[D] = N;  // for printing only
        a[0] = 0;  // for computing
        a[1] = j;
        // b[1] = 1;
        differences[0] = 1;
    }

    inline void step_forward(int t, int &count) {
        const int at = a[t];
        for (int j = 0; j < t; ++j) {
            const int aj = a[j];
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
        for (int j = 0; j < t; ++j) {
            const int aj = a[j];
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
        // int min = 1;
        // if (next == N) {
        //     min = Dp != 0 ? b[Dp] + 1 : b[p];
        // }
        // if (min != 1) return;

        step_forward(D1, count);
        if (count >= N2) {
            printf("\n");
            for (int i = 1; i <= D; ++i) {
                printf("%3d ", a[i]);
            }
            printf("\n");
            fflush(stdout);
        }
        step_backward(D1);
    }

    void GenD(int t, int p, int count) {
        if (t >= D1) {
            PrintD(p, count);
            return;
        }

        const int t_1 = t + 1;
        step_forward(t, count);

        if (count >= N1 + t * t_1 / 2) {
            int tail = ND + t_1;
            const int max = a[t_1 - p] + a[p];

            if (max <= tail) {
                a[t_1] = max;
                // b[t_1] = b[t_1 - p];
                GenD(t_1, p, count);
                tail = max - 1;
            }

            for (int j = tail; j >= a[t] + 1; --j) {
                a[t_1] = j;
                // b[t_1] = 1;
                GenD(t_1, t_1, count);
            }
        }
        step_backward(t);
    }

    void Gen11() {
        int count = 0;
        step_forward(1, count);
        int tail = ND + 2;
        const int max = a[1] + a[1];

        if (max <= tail) {
            a[2] = max;
            // b[2] = b[1];
            GenD(2, 1, count);
            tail = max - 1;
        }

        for (int j = tail; j >= a[1] + 1; --j) {
            a[2] = j;
            // b[2] = 1;
            GenD(2, 2, count);
        }
        // step_backward(1);
    }

    static void usage() { printf("Usage: necklace [n] [d] (n>=3, d>=3, n>=d*(d-1)+1)\n"); }
};

void InitParallel(int N, int D) {
    const unsigned num_workers = std::thread::hardware_concurrency();
    ThreadPool pool(num_workers);
    printf("Number of workers: %u\n", num_workers * 3 / 4);

    std::vector<std::future<void>> results;
    results.reserve((N + 1) / 2 - (N - 1) / D);  // Pre-allocate space

    const int start = (N + 1) / 2;
    const int end = (N - 1) / D + 1;

    for (int j = start; j >= end; --j) {
        results.emplace_back(pool.enqueue([N, D, j]() {
            DcGenerator generator(N, D, j);
            generator.Gen11();
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
