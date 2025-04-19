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

#include <array>
#include <cstdio>
#include <cstdlib>
// #include <future>  // for future
// #include <vector>

// #include "ThreadPool.h"

class NecklaceGenerator {
  private:
    static constexpr int MAX = 20;

    int N = 0;
    int D = 0;

  public:
    std::array<int, MAX> a{};
    std::array<int, MAX> b{};

    NecklaceGenerator(int n, int d) : N(n), D(d) {
        a.fill(0);
        a[D] = N;
    }

    void PrintD(int p) {
        /* Determine minimum position for next bit */
        int next = (D / p) * a[p] + a[D % p];
        if (next < N) return;

        /* Determine last bit */
        int min = 1;
        if ((next == N) && (D % p != 0)) {
            min = b[D % p] + 1;
            p = D;
        } else if ((next == N) && (D % p == 0)) {
            min = b[p];
        }
        int end = N;
        // if (min < 1 || min > 2) {
        //     std::printf("Boom!!!!\n");
        //     std::exit(0);
        // }
        //for (b[D] = min; b[D] < 2; b[D]++) {
        if (min == 1) {
            int i = 1;
            if ((N % a[p] == 0) && (a[p] != N)) {
                // Do nothing special for Lyndon words case
            } else {
                for (int j = 1; j <= end; j++) {
                    if (a[i] == j) {
                        std::printf("%d ", b[i]);
                        i++;
                    } else {
                        std::printf("0 ");
                    }
                }
                std::printf("\n");
            }
            // p = D;
        }
    }

    void GenD(int t, int p) {
        if (t + 1 >= D) {
            PrintD(p);
        } else {
            int tail = N - D + (t + 1);
            int max = a[t - p + 1] + a[p];
            if (max <= tail) {
                a[t + 1] = max;
                b[t + 1] = b[t - p + 1];

                GenD(t + 1, p);
                if (b[t + 1] == 0) {
                    std::printf("b[t + 1] == 0\n");
                    std::exit(0);
                }
        
                // for (int i = b[t + 1] + 1; i < 2; i++) {
                //     b[t + 1] = i;
                //     GenD(t + 1, t + 1);
                // }
                tail = max - 1;
            }
            for (int j = tail; j >= a[t] + 1; j--) {
                a[t + 1] = j;
                b[t + 1] = 1;
                GenD(t + 1, t + 1);
            }
        }
    }

    void Gen11() {
        int tail = N - D + 2;
        int max = a[1] + a[1];
        if (max <= tail) {
            a[2] = max;
            b[2] = b[1];
            if (b[2] == 0) {
                std::printf("b[2] == 0\n");
                std::exit(0);
            }

            GenD(2, 1);
            tail = max - 1;
        }
        for (int j = tail; j >= a[1] + 1; j--) {
            a[2] = j;
            b[2] = 1;
            GenD(2, 2);
        }
    }

    void Init() {
        for (int j = N - D + 1; j >= (N - 1) / D + 1; j--) {
            a[1] = j;
            b[1] = 1;
            Gen11();
        }
    }

    static void usage() { std::printf("Usage: necklace [n] [d] (n>=3, d>=3)\n"); }
};

// void InitParallel(int N, int D) {
//     auto num_workers = std::thread::hardware_concurrency() * 3 / 4;
//     ThreadPool pool(num_workers);
//     printf("Number of workers: %d\n", num_workers);
//     std::vector<std::future<void>> results;
//     auto start = (N + 1) / 2;
//     auto end = (N - 1) / D + 1;

//     // for (auto j = num_elem - density + 1; j >= end; j--) {
//     for (auto j = start; j >= end; j--) {
//         results.emplace_back(pool.enqueue([&N, &D, j]() {
//             NecklaceGenerator generator(N, D);
//             // generator.a.fill(0);
//             // generator.a[D] = N;
//             generator.a[1] = j;
//             generator.b[1] = 1;
//             generator.Gen11();
//         }));
//     }
//     auto countdown = start - end;
//     for (auto &&result : results) {
//         // printf("%3d\r", countdown--);
//         fflush(stdout);
//         result.get();
//     }
//     printf("\n");
// }

int main(int argc, char **argv) {
    if (argc != 3) {
        NecklaceGenerator::usage();
        return 1;
    }

    int N, D;
    std::sscanf(argv[1], "%d", &N);

    if (N < 3) {
        NecklaceGenerator::usage();
        return 1;
    }

    std::sscanf(argv[2], "%d", &D);
    if (D < 3) {
        NecklaceGenerator::usage();
        return 1;
    }

    // NECKLACES                                LYNDON WORDS
    // -------------                            -------------
    // 2.  Fixed-density                        22. Fixed-density
    NecklaceGenerator generator(N, D);
    generator.Init();
    std::printf("Finish sucessfully\n");
    return 0;
}
