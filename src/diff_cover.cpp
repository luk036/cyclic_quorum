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
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring> // for memset, memcpy
#include <future>  // for future
#include <vector>

#include "ThreadPool.h"

static constexpr int MAX_N = 160;

class DcGenerator {
  private:
    static constexpr int MAX = 20;

    const int N;
    const int D;
    const int ND;
    const int N1;

    std::array<int, MAX> a{};
    std::array<int, MAX> b{};
    std::array<int8_t, MAX_N> differences{};
    int count;

  public:
    DcGenerator(int n, int d, int j) : N{n}, D{d}, ND{N - D}, N1{N - D * (D - 1)}, count{1} {
        this->a.fill(0);
        this->a[this->D] = this->N; // for sake of printing only
        this->a[0] = 0; // for sake of computing
        this->a[1] = j;
        this->b[1] = 1;
        this->differences.fill(0);
        this->differences[0] = 1;
    }

    void step_foreward(int t) {
        for (int j = 0; j < t; j++) {
            const auto p_diff = this->a[t] - this->a[j];
            const auto n_diff = this->N - p_diff;
            // Note that p_diff may be equal to n_diff
            if (this->differences[p_diff] == 0) {
                this->count++;
            }
            this->differences[p_diff]++;
            if (this->differences[n_diff] == 0) {
                this->count++;
            }
            this->differences[n_diff]++;
        }
    }

    void step_backward(int t) {
        for (int j = 0; j < t; j++) {
            const auto p_diff = this->a[t] - this->a[j];
            const auto n_diff = this->N - p_diff;
            // Note that p_diff may be equal to n_diff
            this->differences[p_diff]--;
            if (this->differences[p_diff] == 0) {
                this->count--;
            }
            this->differences[n_diff]--;
            if (this->differences[n_diff] == 0) {
                this->count--;
            }
        }
    }

    void PrintD(const int p) {
        /* Determine minimum position for next bit */
        const int next = (this->D / p) * this->a[p] + this->a[this->D % p];
        if (next < this->N) return;

        /* Determine last bit */
        int min = 1;
        if ((next == this->N) && (this->D % p != 0)) {
            min = this->b[this->D % p] + 1;
        } else if ((next == this->N) && (this->D % p == 0)) {
            min = this->b[p];
        }
        const int end = this->N;
        if (min == 1) {
            if ((this->N % this->a[p] == 0) && (this->a[p] != this->N)) {
                // Do nothing special for Lyndon words case
            } else {
                this->step_foreward(D - 1);
                if (this->count >= this->N) {
                    printf("\n");
                    for (auto i = 1; i <= this->D; i++) {
                        printf("%3d ", this->a[i]);
                    }
                    printf("\n");
                    fflush(stdout);
            
                    // int i = 1;
                    // for (int j = 1; j <= end; j++) {
                    //     if (this->a[i] == j) {
                    //         std::printf("%d ", this->b[i]);
                    //         i++;
                    //     } else {
                    //         std::printf("0 ");
                    //     }
                    // }
                    // std::printf("\n");
                }
                this->step_backward(D - 1);
            }
        }
    }

    void GenD(const int t, const int p) {
        if (t + 1 >= this->D) {
            this->PrintD(p);
        } else {
            this->step_foreward(t);
            if (this->count >= this->N1 + t * (t + 1)) {
                int tail = this->ND + (t + 1);
                const int max = this->a[t + 1 - p] + this->a[p];
                if (max <= tail) {
                    this->a[t + 1] = max;
                    this->b[t + 1] = this->b[t + 1 - p];
                    this->GenD(t + 1, p);

                    tail = max - 1;
                }
                for (int j = tail; j >= this->a[t] + 1; j--) {
                    this->a[t + 1] = j;
                    this->b[t + 1] = 1;
                    this->GenD(t + 1, t + 1);
                }
            }
            this->step_backward(t);
        }
    }

    void Gen11() {
        this->step_foreward(1);
        int tail = this->ND + 2;
        const int max = this->a[1] + this->a[1];
        if (max <= tail) {
            this->a[2] = max;
            this->b[2] = this->b[1];
            this->GenD(2, 1);

            tail = max - 1;
        }
        for (int j = tail; j >= this->a[1] + 1; j--) {
            this->a[2] = j;
            this->b[2] = 1;
            this->GenD(2, 2);
        }
        this->step_backward(1);
    }

    static void usage() { std::printf("Usage: necklace [n] [d] (n>=3, d>=3, n>=d*(d-1)+1)\n"); }
};

void InitParallel(int N, int D) {
    const auto num_workers = std::thread::hardware_concurrency() * 3 / 4;
    ThreadPool pool(num_workers);
    printf("Number of workers: %u\n", num_workers);
    std::vector<std::future<void>> results;
    const auto start = (N + 1) / 2;
    const auto end = (N - 1) / D + 1;

    for (auto j = start; j >= end; j--) {
        results.emplace_back(pool.enqueue([&N, &D, j]() {
            DcGenerator generator(N, D, j);
            generator.Gen11();
        }));
    }
    auto countdown = start - end;
    for (auto &&result : results) {
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

    int N, D;
    std::sscanf(argv[1], "%d", &N);

    if (N < 3) {
        DcGenerator::usage();
        return 1;
    }

    std::sscanf(argv[2], "%d", &D);
    if (D < 3) {
        DcGenerator::usage();
        return 1;
    }

    if (N > D * (D - 1) + 1) {
        printf("Error: N must be less than or equal to D * (D - 1) + 1\n");
        return 1;
    }

    InitParallel(N, D);
    std::printf("Finish sucessfully\n");
    return 0;
}