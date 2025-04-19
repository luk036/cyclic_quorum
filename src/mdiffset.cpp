/**
mdiffset.cpp

This code is designed to generate and explore difference covers, which are
special sets of numbers used in various mathematical applications. The program
takes three main inputs from the command line: the total number of elements
(num_elem), the density of the difference cover (density), and a threshold
value.

The main purpose of this code is to find all possible difference covers that
meet certain criteria based on the input parameters. It does this by
systematically building up these sets one element at a time and checking if they
satisfy the required conditions.

The program starts by checking if the correct number of command-line arguments
is provided. If not, it displays a usage message and exits. It then reads the
input values and performs a quick check to ensure the number of elements isn't
too large compared to the density.

To speed up the process of finding difference covers, the program uses parallel
processing. It creates a "thread pool" that allows multiple calculations to run
simultaneously on different CPU cores. This is especially helpful when dealing
with large numbers or complex sets.

The main algorithm works by creating multiple DiffCover objects, each
representing a potential difference cover. For each of these objects, it sets
some initial values and then calls a function named GenD. This GenD function is
where the real work happens - it recursively builds up the difference cover,
checking at each step whether the current set of numbers meets the required
conditions.

As the program runs, it keeps track of how many calculations are left to do and
displays a countdown. When all calculations are complete, the program finishes.

The most important part of the logic is in the GenD function. This function
works by trying different possibilities for each element in the set. It keeps
track of the differences between elements using an array, and uses this
information to decide whether to continue exploring a particular set of numbers
or to backtrack and try something else.

In simpler terms, you can think of this program as exploring a huge tree of
possibilities. Each branch of the tree represents a different potential
difference cover. The program systematically checks each branch, pruning off
ones that won't work and following promising ones to their conclusion. When it
finds a valid difference cover, it prints it out.

The main output of this program is a series of difference covers printed to the
console. These are the sets of numbers that satisfy all the conditions based on
the input parameters.

Overall, this code demonstrates an efficient way to solve a complex mathematical
problem by breaking it down into smaller parts, using parallel processing to
speed things up, and systematically exploring all possibilities.
*/
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>  // for future
#include <vector>

#include "ThreadPool.h"

const auto MAX = 20;
const auto MAX_N = 80;

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------
struct DiffCover {
    int num_elem;
    int density;
    int a[MAX];
    int b[MAX];
    int threshold;
    int d_minus_1;
    int d_times_d_minus_1;
    int n_minus_d;
    int n1;
    int n2;
    // const int *begin_a;
    size_t size_n;

    /**
     * Constructs a DiffCover object with the given parameters.
     *
     * @param num_elem The total number of elements.
     * @param density The number of elements in the D-set.
     * @param threshold The threshold for the number of differences between
     * elements.
     */
    DiffCover(int num_elem, int density, int threshold)
        : num_elem{num_elem},
          density{density},
          threshold{threshold},
          d_minus_1{density - 1},
          d_times_d_minus_1{density * (density - 1)},
          n_minus_d{num_elem - density},
          n1{num_elem / 2 - density * (density - 1) / 2},
          n2{num_elem / 2},  // begin_a{&a[0]},
          size_n{(num_elem / 2 + 1) * sizeof(int8_t)} {
        for (auto j = 0; j <= density; j++) a[j] = 0;

        a[density] = num_elem;
        a[0] = 0;  // for convenience
    }

    //-------------------------------------------------------------
    /**
     * Prints the generated D-set to stdout.
     */
    void PrintD() const {
        // print a
        printf("\n");
        for (auto i = 1; i <= this->density; i++) {
            printf("%d ", this->a[i]);
        }
        printf("\n");
        fflush(stdout);
        return;
    }

    /**
     * GenD Function
     *
     * The GenD function is a recursive algorithm designed to generate
     * difference covers, which are special sets of numbers used in various
     * mathematical applications. This function is part of a larger program that
     * explores different possible difference covers based on certain input
     * parameters.
     *
     * The function takes four inputs: 't' (the current element index being
     * added), 'p' (the previous element index), 'tt' (a triangle number related
     * to 't'), and 'diffset' (an array tracking differences between elements).
     *
     * The main output of this function is not directly returned, but rather it
     * prints the generated difference cover when a valid set is found using the
     * PrintD method.
     *
     * The function works by building up the difference cover one element at a
     * time. It starts by copying the input diffset into a new array called
     * 'differences'. Then, it calculates new differences based on the current
     * element and updates the 'differences' array.
     *
     * There's a threshold check that counts the number of differences and
     * compares it to a calculated value. If the count is too low, the function
     * returns early, as this branch won't lead to a valid difference cover.
     *
     * If the function hasn't returned early, it then checks if it has reached
     * the end of the set (when t1 >= this->density). If so, it prints the
     * current difference cover using PrintD.
     *
     * If it hasn't reached the end, the function calculates some new values and
     * enters a recursive phase. It tries different possibilities for the next
     * element in the set, calling itself recursively for each possibility. This
     * is how it explores all potential difference covers.
     *
     * The logic flow involves a lot of conditional checks and recursive calls.
     * The function is essentially performing a depth-first search through the
     * space of possible difference covers, pruning branches that won't lead to
     * valid solutions.
     *
     * An important data transformation happening is the continuous updating of
     * the 'differences' array, which keeps track of the differences between
     * elements in the current partial solution. This array is crucial for
     * determining whether a particular branch of the search is worth pursuing.
     *
     * In simple terms, you can think of this function as exploring a tree of
     * possibilities, where each node in the tree represents a partial
     * difference cover. The function keeps adding elements and checking if they
     * lead to a valid solution, backtracking when necessary to try other
     * possibilities.
     */

    /*-----------------------------------------------------------*/
    // FIXED DENSITY
    /*-----------------------------------------------------------*/
    /**
     * Recursively generates all possible D-sets by building them up one
     * element at a time.
     *
     * @param t - Current element index being added
     * @param p - Previous element index
     * @param tt - Triangle number of current index t
     * @param diffset[] - Bit array tracking differences between elements
     */
    void GenD(int t, int p, int tt, int8_t diffset[]) {
        int8_t differences[MAX_N];
        memcpy(differences, diffset, this->size_n);

        const auto at = this->a[t];
        for (auto ptr = &this->a[0]; ptr != &this->a[0] + t; ++ptr) {
            const auto p_diff = at - *ptr;
            const auto n_diff = this->num_elem - p_diff;
            differences[p_diff <= n_diff ? p_diff : n_diff] = 1;
        }
        if (t >= this->threshold) {
            int8_t count = 0;
            const int8_t *begin = &differences[1];
            for (auto ptr = begin; ptr != begin + this->n2; ++ptr) {
                count += *ptr;
            }
            if (int(count) < this->n1 + tt) {
                return;
            }
        }

        const auto t1 = t + 1;
        if (t1 >= this->density)
            this->PrintD();
        else {
            auto tail = this->n_minus_d + t1;
            const auto max = this->a[t1 - p] + this->a[p];
            const auto tt1 = t1 * (t1 + 1) / 2;
            if (max <= tail) {
                this->a[t1] = max;
                this->b[t1] = this->b[t1 - p];

                this->GenD(t1, p, tt1, differences);
                if (this->b[t1] == 0) {
                    this->b[t1] = 1;
                    this->GenD(t1, t1, tt1, differences);
                }
                tail = max - 1;
            }
            for (auto j = tail; j >= at + 1; j--) {
                this->a[t1] = j;
                this->b[t1] = 1;
                this->GenD(t1, t1, tt1, differences);
            }
        }
    }
};

//------------------------------------------------------
void usage() { printf("Usage: necklace [num_elem] [density] [threshold]\n"); }
//--------------------------------------------------------------------------------

/**
 * Main Function in mdiffset.cpp
 *
 * This code is the main function of a program that generates difference covers,
 * which are special sets of numbers used in various mathematical and
 * computational applications. The program takes command-line arguments, uses
 * parallel processing to speed up calculations, and prints the results.
 *
 * The function takes two inputs: the number of command-line arguments (argc)
 * and an array of those arguments (argv). It expects three specific arguments:
 * num_elem (the size of the set), density (the density of the difference
 * cover), and a threshold value.
 *
 * The main output of this program is a series of difference covers printed to
 * the console. These are sets of numbers that satisfy certain mathematical
 * properties based on the input parameters.
 *
 * Here's how the program works:
 *
 * It first checks if the correct number of arguments is provided. If not, it
 * shows a usage message and exits.
 *
 * It then reads the input values (num_elem, density, and threshold) from the
 * command-line arguments.
 *
 * There's a check to ensure that num_elem is not too large compared to density.
 * If it is, the program prints an error message and exits.
 *
 * The program then sets up parallel processing. It creates a "thread pool" with
 * a number of workers based on the computer's capabilities. This allows the
 * program to run calculations simultaneously on multiple CPU cores.
 *
 * The main algorithm runs in a loop. For each value of j from start to end:
 *
 * It creates a new DiffCover object with the input parameters.
 * It sets initial values for this object.
 * It calls a function named GenD, which likely generates the difference cover.
 * Each of these tasks is added to the thread pool to be executed in parallel.
 * Finally, the program waits for all the parallel tasks to complete, showing a
 * countdown as they finish.
 *
 * The key logic in this code is the parallelization of the DiffCover
 * generation. Instead of calculating one difference cover at a time, it starts
 * multiple calculations simultaneously, potentially speeding up the overall
 * process significantly. The actual generation of the difference covers happens
 * in the GenD function, which is called for each parallel task.
 *
 * This program is designed to efficiently explore many possible difference
 * covers, leveraging parallel processing to speed up what could otherwise be a
 * time-consuming mathematical computation.
 */
int main(int argc, char **argv) {
    if (argc < 4) {
        usage();
        return 1;
    }
    int num_elem, density, threshold;

    sscanf(argv[1], "%d", &num_elem);
    sscanf(argv[2], "%d", &density);
    sscanf(argv[3], "%d", &threshold);

    if (num_elem > density * (density - 1) + 1) {
        printf("Error: N must be less than or equal to D * (D - 1) + 1\n");
        return 1;
    }

    // DiffCover diff_cover(num_elem, density, threshold);
    // printf("%3d\n", end);
    // diff_cover.run();

    auto num_workers = std::thread::hardware_concurrency() * 3 / 4;
    ThreadPool pool(num_workers);
    printf("Number of workers: %d\n", num_workers);
    std::vector<std::future<void>> results;
    auto start = (num_elem + 1) / 2;
    auto end = (num_elem - 1) / density + 1;

    // for (auto j = num_elem - density + 1; j >= end; j--) {
    for (auto j = start; j >= end; j--) {
        results.emplace_back(pool.enqueue([&num_elem, &density, &threshold, j]() {
            DiffCover dc(num_elem, density, threshold);
            dc.a[1] = j;
            dc.b[1] = 1;
            int8_t differences[MAX_N];
            memset(differences, 0, dc.size_n);
            differences[0] = 1;
            dc.GenD(1, 1, 1, differences);
        }));
    }
    auto countdown = start - end;
    for (auto &&result : results) {
        printf("%3d\r", countdown--);
        fflush(stdout);
        result.get();
    }
    printf("\n");
    return 0;
}
