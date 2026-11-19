/**
 * Threaded min-conflicts local search for a relaxed cyclic (N, D) difference
 * cover: pick D elements of Z_N whose pairwise differences cover every nonzero
 * residue.
 *
 * Each worker thread runs independent random restarts; a conflict is an
 * uncovered residue, and a move swaps one chosen element for a candidate that
 * covers it. The search is incomplete: a set it prints has been verified to
 * cover every residue, but finding nothing is not a proof that nothing exists.
 *
 * Usage: ./cover_ls <N> <D> [seconds] [threads]
 *
 * Environment:
 *   LS_SEED=n        shift the fixed seed family (reproducible per n)
 *   LS_START="a b c" start every restart from a known cover; with D+1 values one
 *                    is dropped, which is what makes N near the top of the range
 *                    reachable at all
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace {

// Default number of worker threads.
constexpr int kDefaultThreads = 10;

// Number of restarts without progress before a worker starts over.
constexpr int kStallLimit = 300;

// A worker's private search state, so threads share nothing but the atomics.
struct Board {
    const int N;
    const int D;
    std::vector<int> cnt;      // cnt[r] = how often residue r is produced
    std::vector<int> S;        // current D-element set
    std::vector<int> unclist;  // residues with cnt == 0
    std::vector<char> in;      // in[x] = 1 iff x is in S
    std::vector<int> hint;     // optional warm-start cover, used as a (D+1)-set
    std::mt19937 rng;

    Board(int n, int d, unsigned seed) : N(n), D(d), cnt(n, 0), S(d), in(n, 0), rng(seed) {}

    void AddPair(int a, int b, int delta) {
        const int d = (a - b + N) % N;
        if (d) {
            cnt[d] += delta;
            cnt[N - d] += delta;
        }
    }

    // Recompute the coverage counts from S and refresh the conflict list.
    int Rebuild() {
        std::fill(cnt.begin(), cnt.end(), 0);
        for (int i = 0; i < D; ++i)
            for (int j = i + 1; j < D; ++j) AddPair(S[i], S[j], +1);
        unclist.clear();
        for (int r = 1; r < N; ++r)
            if (!cnt[r]) unclist.push_back(r);
        return static_cast<int>(unclist.size());
    }

    void RandomStart() {
        std::fill(in.begin(), in.end(), 0);
        if (hint.size() == static_cast<size_t>(D) + 1) {
            // Warm start: drop one element of a known (D+1)-element cover, so
            // the repair only has to undo a single deletion.
            const size_t skip = rng() % hint.size();
            size_t k = 0;
            for (size_t i = 0; i < hint.size(); ++i) {
                if (i == skip) continue;
                S[k++] = hint[i];
                in[hint[i]] = 1;
            }
            return;
        }
        for (int i = 0; i < D; ++i) {
            int x;
            do {
                x = rng() % N;
            } while (in[x]);
            in[x] = 1;
            S[i] = x;
        }
    }

    // Swap S[i] -> x, returning the resulting number of uncovered residues and
    // leaving the counts consistent with the new set.
    int ApplySwap(int i, int x) {
        const int s = S[i];
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(s, S[j], -1);
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(x, S[j], +1);
        in[s] = 0;
        in[x] = 1;
        S[i] = x;
        unclist.clear();
        for (int r = 1; r < N; ++r)
            if (!cnt[r]) unclist.push_back(r);
        return static_cast<int>(unclist.size());
    }

    // Would-be result of swapping S[i] -> x, with the state left unchanged.
    // Cheaper than ApplySwap twice: no full rebuild, one scan for conflicts.
    int EvaluateSwap(int i, int x) {
        const int s = S[i];
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(s, S[j], -1);
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(x, S[j], +1);
        int u = 0;
        for (int r = 1; r < N; ++r)
            if (!cnt[r]) ++u;
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(x, S[j], -1);
        for (int j = 0; j < D; ++j)
            if (j != i) AddPair(s, S[j], +1);
        return u;
    }
};

void Worker(int N, int D, std::atomic<bool> &found, std::mutex &out, std::atomic<int> &best,
            const std::vector<int> &start, unsigned seed,
            std::chrono::steady_clock::time_point deadline) {
    Board board(N, D, seed);
    board.hint = start;
    auto expired = [&]() { return std::chrono::steady_clock::now() > deadline; };

    while (!found && !expired()) {
        board.RandomStart();
        int cur = board.Rebuild();
        int stall = 0;
        int ticks = 0;
        // Reverse-move tabu: without it the search can swap a value in, then
        // straight back out, cycling between two states with one class missing.
        int tabuI = -1, tabuX = -1, tabuUntil = 0;

        while (cur > 0 && !found) {
            if (cur < best.load()) best.store(cur);
            if (++ticks % 256 == 0 && expired()) return;

            const int r = board.unclist[board.rng() % board.unclist.size()];
            int bestI = -1, bestX = -1, bestUnc = cur + 1000;
            for (int i = 0; i < D; ++i) {
                const int s = board.S[i];
                for (int sgn = 0; sgn < 2; ++sgn) {
                    const int x = (s + (sgn ? r : N - r)) % N;
                    if (board.in[x]) continue;
                    if (i == tabuI && x == tabuX && ticks < tabuUntil) continue;
                    const int after = board.EvaluateSwap(i, x);
                    if (after < bestUnc) {
                        bestUnc = after;
                        bestI = i;
                        bestX = x;
                    }
                }
            }
            if (r == N / 2 && N % 2 == 0) {
                // The antipodal class needs a pair (t, t + N/2); the same-index
                // move cannot create one, so anchor on t and replace another
                // element. This is the only move that closes that class.
                for (int j = 0; j < D; ++j) {
                    const int x = (board.S[j] + N / 2) % N;
                    if (board.in[x]) continue;
                    for (int i = 0; i < D; ++i) {
                        if (i == j) continue;
                        if (i == tabuI && x == tabuX && ticks < tabuUntil) continue;
                        const int after = board.EvaluateSwap(i, x);
                        if (after < bestUnc) {
                            bestUnc = after;
                            bestI = i;
                            bestX = x;
                        }
                    }
                }
            }
            if (bestI < 0) {  // no candidate covers r; keep the restart moving
                ++stall;
                if (stall > kStallLimit) break;
                continue;
            }
            const bool improved = bestUnc < cur;
            tabuI = bestI;
            tabuX = board.S[bestI];
            tabuUntil = ticks + 4 + static_cast<int>(board.rng() % static_cast<unsigned>(D));
            cur = board.ApplySwap(bestI, bestX);
            if (improved) {
                stall = 0;
            } else if (++stall > kStallLimit) {
                break;
            }
        }

        if (cur == 0) {
            std::lock_guard<std::mutex> lock(out);
            if (!found.exchange(true)) {
                std::vector<int> R = board.S;
                std::sort(R.begin(), R.end());
                const int base = R[0];
                printf("%d:", N);
                for (int v : R) printf("%4d", (v - base + N) % N);
                printf("   (d=%d)\n", D);
                fflush(stdout);
            }
            return;
        }
    }
}

}  // namespace

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: cover_ls <N> <D> [seconds] [threads]\n");
        return 1;
    }
    const int N = atoi(argv[1]);
    const int D = atoi(argv[2]);
    const double secs = (argc > 3) ? atof(argv[3]) : 10.0;
    const int nthreads = (argc > 4) ? atoi(argv[4]) : kDefaultThreads;

    if (N < 3 || D < 2 || D >= N || static_cast<long long>(D) * (D - 1) + 1 < N) {
        fprintf(stderr, "Invalid parameters: need 3 <= N and d(d-1)+1 >= N\n");
        return 1;
    }
    if (nthreads < 1) {
        fprintf(stderr, "Invalid thread count\n");
        return 1;
    }

    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(static_cast<long long>(secs * 1000));

    std::atomic<bool> found(false);
    std::atomic<int> best(N);
    std::mutex out;
    // LS_START="v1 v2 ..." seeds every worker from a known cover; with D elements
    // it is used as is, with D+1 one element is dropped at each restart.
    std::vector<int> start;
    if (const char *s = std::getenv("LS_START")) {
        const char *p = s;
        while (*p) {
            char *end = nullptr;
            const long v = std::strtol(p, &end, 10);
            if (end == p) {
                ++p;
                continue;
            }
            if (v >= 0 && v < N) start.push_back(static_cast<int>(v));
            p = end;
        }
    }
    // The seeds are fixed by default so a run is reproducible; LS_SEED shifts
    // them, which is the cheapest way to explore a different slice of starts.
    const char *seedEnv = std::getenv("LS_SEED");
    const unsigned base =
        seedEnv ? 20261114u + 104729u * static_cast<unsigned>(atoi(seedEnv)) : 20261114u;
    std::vector<std::thread> workers;
    workers.reserve(nthreads);
    for (int i = 0; i < nthreads; ++i) {
        workers.emplace_back(Worker, N, D, std::ref(found), std::ref(out), std::ref(best),
                             std::cref(start), base + 7919u * static_cast<unsigned>(i), deadline);
    }
    for (auto &t : workers) t.join();

    if (!found) {
        fprintf(stderr, "none: N=%d D=%d, best uncovered=%d, threads=%d\n", N, D, best.load(),
                nthreads);
        return 1;
    }
    return 0;
}
