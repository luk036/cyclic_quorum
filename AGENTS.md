# AGENTS.md — Cyclic Quorum

Research codebase exploring cyclic quorum systems for distributed mutual exclusion.
Two approaches: **combinatorial search** (backtracking + thread pool) and **deep RL** (policy gradient).

## Quick start

```bash
# Combinatorial search (find optimal difference covers for N=112, D=12)
g++ -std=c++11 -pthread -O3 -o diff_cover src/diff_cover.cpp
./diff_cover 112 12

# Multi-threaded RL (REINFORCE with 3-layer network, 10 threads)
g++ -std=c++11 -pthread -O3 -o rl_cover src/RL_mt.cpp
./rl_cover 112 12
```

- C++ files: `g++ -std=c++11 -pthread -O3`
- C files (necklace/bracelet generators): `gcc -O3`
- No Makefile, no CMakeLists.txt — always manual compile
- `-march=native` safe to add (used historically)

## Source layout

| Path | Role |
|---|---|
| `src/diff_cover.cpp` | Main combinatorial search (`DcGenerator` + `ThreadPool`) |
| `src/RL_mt.cpp` | Main RL solver (10 threads, REINFORCE, 3-layer NN) |
| `src/ThreadPool.h` | Single-header thread pool (used by all C++ search programs) |
| `src/bracelet*.cpp`, `src/bracelets.c`, `src/necklace*.c`, `src/lyndon.c` | Necklace/bracelet generators (Joe Sawada, symmetry breaking) |
| `src/diff_cover2.cpp`, `src/diff_cover3.cpp`, `src/bdiffcover.cpp`, `src/mdiffset.cpp` | Variants with different pruning / symmetry strategies |
| `src/RL.cpp` | Single-threaded RL (simpler, good for understanding) |
| `paper/` | LaTeX paper (`cqs.tex`, `cqs.md`, `cyclic_quorum.md`) |
| `refs/` | Reference PDFs (don't modify) |
| `*.md` at root | `project-intro.md` (slide deck), `fyproj2026.md` (thesis proposal, Chinese), `IFLOW.md` (iFlow context, Chinese) |

## Key constraints

- `N ≥ 3`, `D ≥ 3`, and **`N ≤ D*(D-1)+1`** — enforced at startup in every solver
- `MAX_D = 20` in search algorithms (hard-coded array size)
- `MAX_C = 128` for difference tracking arrays (`diff_cover.cpp`)

## Architecture notes

- **diff_cover.cpp**: Recursive backtracking in `GenD()`, prunes when `count < N1 + t*(t+1)/2`. Range of starting values `[(N-1)/D + 1, (N+1)/2]` mapped to thread pool workers.
- **RL_mt.cpp**: Policy network `2N → 256 → 128 → N`, Xavier init, `γ=0.98`, `lr=0.01`, `MAX_EPISODES=1e9`. Shared network mutex-protected. RL always fixes element 0 first.
- Symmetry breaking in search uses bracelet generation (Joe Sawada's algorithm). C files (`bracelets.c`, `necklace.c`) are the reference implementations; C++ files are ports.

## Code style

- `.clang-format` is **Google style** with: `BreakBeforeBraces: Attach`, `ColumnLimit: 100`, `IndentWidth: 4`, `IncludeBlocks: Regroup`
- All C++ is C++11 (no C++14/17 features beyond what the conditional `__cplusplus` checks guard)
- Copyright headers: GPL v2, Joe Sawada (2019) on most files
- Variable naming: short (a, b, N, D, n, d, t, p) — mathematical notation, not descriptive

## Files you should NOT edit

- `.git/opencode` — stores commit hash, do not touch
- `.rumdl_cache/` — markdown linter cache, auto-generated
- `refs/` — reference PDFs
- `src/output.txt`, `src/golden.txt`, `src/diffcover.txt` — result output, not source

## Markdown linting

- Config: `rumdl.toml` with line length 1200
- Run: `rumdl check .`
- Used historically to enforce markdown quality

## Verification (no test framework)

- Search correctness is checked by: output must satisfy the difference cover property (every residue 0..N-1 is covered)
- `src/golden.txt` contains known-good results for spot-checking
- No unit tests, no CI — manual verification against known results
