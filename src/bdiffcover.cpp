// #include "ThreadPool.h"
// #include <cstdint>
// #include <cstdlib>
// #include <cstring>
// #include <future> // for future
// #include <vector>
#include <cstdio>

static const int MAX = 20;
static const int MAX_N = 160;
static const int NECK = 1;
static const int LYN = 0;
static const int TRUE = 1;
static const int FALSE = 0;

struct cell {
    int next, prev;
};

struct element {
    int s, v;
};

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------
struct DiffCover {
    element B[MAX_N];
    cell avail[MAX_N];
    int nb = 0;
    // run length encoding data structure
    // number of blocks
    int num[3];
    int a[MAX_N];
    int run[MAX_N];
    int n;
    int k = 2;
    int total;
    int head;
    int threshold;

    /**
     * Constructs a DiffCover object with the given parameters.
     *
     * @param n The total number of elements.
     * @param d The number of elements in the set (density).
     * @param threshold The threshold for the number of differences between
     * elements.
     */
    DiffCover(int n, int d, int threshold) : n{n}, threshold{threshold} {
        this->num[1] = this->n - d;
        this->num[2] = d;
        for (int idx = this->k + 1; idx >= 0; idx--) {
            this->avail[idx].next = idx - 1;
            this->avail[idx].prev = idx + 1;
        }
        this->head = this->k;
        for (int idx = 1; idx <= this->n; idx++) {
            this->a[idx] = this->k;
            this->run[idx] = 0;
        }
        this->total = 0;
        this->a[1] = 1;
        this->num[1]--;
        if (this->num[1] == 0)
            this->ListRemove(1);
        this->B[0].s = 0;
        this->UpdateRunLength(1);
    }

    /*-----------------------------------------------------------*/
    void ListRemove(int i) {
        int p, n;
        if (i == this->head)
            this->head = this->avail[i].next;
        p = this->avail[i].prev;
        n = this->avail[i].next;
        this->avail[p].next = n;
        this->avail[n].prev = p;
    }

    void ListAdd(int i) {
        int p, n;
        p = this->avail[i].prev;
        n = this->avail[i].next;
        this->avail[n].prev = i;
        this->avail[p].next = i;
        if (this->avail[i].prev == this->k + 1)
            this->head = i;
    }

    int ListNext(int i) { return this->avail[i].next; }

    /*-----------------------------------------------------------*/
    void Print(int p) {
        int idx;
        if (this->n != p)
            return;
        for (idx = 1; idx <= this->n; idx++)
            printf("%d ", this->a[idx] - 1);
        printf("\n");
        this->total++;
    }
    /*-----------------------------------------------------------*/
    void UpdateRunLength(int v) {
        if (this->B[this->nb].s == v)
            this->B[this->nb].v = this->B[this->nb].v + 1;
        else {
            this->nb++;
            this->B[this->nb].v = 1;
            this->B[this->nb].s = v;
        }
    }
    void RestoreRunLength() {
        if (this->B[this->nb].v == 1)
            this->nb--;
        else
            this->B[this->nb].v = this->B[this->nb].v - 1;
    }
    /*---------------------------------------------------------------------*/
    // return-1 if reverse smaller, 0 if equal, and 1 if reverse is larger
    /*---------------------------------------------------------------------*/
    int CheckRev() {
        int idx;
        idx = 1;
        while (this->B[idx].v == this->B[this->nb - idx + 1].v &&
               this->B[idx].s == this->B[this->nb - idx + 1].s && idx <= this->nb / 2)
            idx++;
        if (idx > this->nb / 2)
            return 0;
        if (this->B[idx].s < this->B[this->nb - idx + 1].s)
            return 1;
        if (this->B[idx].s > this->B[this->nb - idx + 1].s)
            return -1;
        if (this->B[idx].v < this->B[this->nb - idx + 1].v &&
            this->B[idx + 1].s < this->B[this->nb - idx + 1].s)
            return 1;
        if (this->B[idx].v > this->B[this->nb - idx + 1].v &&
            this->B[idx].s < this->B[nb - idx].s)
            return 1;
        return -1;
    }

    /*-----------------------------------------------------------*/
    void Gen(int t, int p, int r, int z, int b, int RS) {
        int j, z2, p2, c;
        // Incremental comparison of a[r+1...n] with its reversal
        if (t - 1 > (this->n - r) / 2 + r) {
            if (this->a[t - 1] > this->a[this->n - t + 2 + r])
                RS = FALSE;
            else if (this->a[t - 1] < this->a[this->n - t + 2 + r])
                RS = TRUE;
        }
        // Termination condition- only characters k remain to be appended
        if (this->num[k] == this->n - t + 1) {
            if (this->num[k] > this->run[t - p])
                p = n;
            if (this->num[k] > 0 && t != r + 1 && this->B[b + 1].s == this->k &&
                this->B[b + 1].v > this->num[this->k])
                RS = TRUE;
            if (this->num[k] > 0 && t != r + 1 &&
                (this->B[b + 1].s != this->k ||
                 this->B[b + 1].v < this->num[this->k]))
                RS = FALSE;
            if (RS == FALSE)
                Print(p);
        }
        // Recursively extend the prenecklace- unless only 0s remain to be
        // appended
        else if (this->num[1] != this->n - t + 1) {
            j = this->head;
            while (j >= this->a[t - p]) {
                this->run[z] = t - z;
                this->UpdateRunLength(j);
                this->num[j]--;
                if (this->num[j] == 0)
                    this->ListRemove(j);
                this->a[t] = j;
                z2 = z;
                if (j != this->k)
                    z2 = t + 1;
                p2 = p;
                if (j != this->a[t - p])
                    p2 = t;
                c = this->CheckRev();
                if (c == 0)
                    this->Gen(t + 1, p2, t, z2, this->nb, FALSE);
                if (c == 1)
                    this->Gen(t + 1, p2, r, z2, b, RS);
                if (this->num[j] == 0)
                    this->ListAdd(j);
                this->num[j]++;
                this->RestoreRunLength();
                j = this->ListNext(j);
            }
            this->a[t] = this->k;
        }
    }

    /*-----------------------------------------------------------*/
    void Gen0() {
        //        2      1      1      2      1      FALSE
        // Incremental comparison of a[r+1...n] with its reversal
        // Termination condition- only characters k remain to be appended
        // Recursively extend the prenecklace- unless only 0s remain to be
        // appended
        for (int idx = this->head; idx >= this->a[1]; idx = this->ListNext(idx)) {
            int z2, p2, c;

            this->run[2] = 0;
            this->UpdateRunLength(idx);
            this->num[idx]--;
            if (this->num[idx] == 0)
                this->ListRemove(idx);
            this->a[2] = idx;
            z2 = 2;
            if (idx != this->k)
                z2 = 3;
            p2 = 1;
            if (idx != a[1])
                p2 = 2;
            c = this->CheckRev();
            if (c == 0) {
                printf("c == 0\n");
                this->Gen(3, p2, 2, z2, this->nb, FALSE);
            }
            if (c == 1) {
                printf("c == 1\n");
                this->Gen(3, p2, 1, z2, 1, FALSE);
            }
            if (this->num[idx] == 0)
                this->ListAdd(idx);
            this->num[idx]++;
            this->RestoreRunLength();

        }
        this->a[2] = this->k;
    }
};

//------------------------------------------------------
void usage() { printf("Usage: bdiffcover [num_elem] [density] [threshold]\n"); }
//--------------------------------------------------------------------------------

/*-----------------------------------------------------------*/
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
        printf("Error: N must be less than D*(D-1)+1\n");
        return 1;
    }

    // int j;
    // int n;
    // int k = 2; // 2
    // int num[3];
    // int d;

    // printf("enter n d: ");
    // scanf("%d %d", &n, &d);
    // for (int j = 1; j <= k; j++) {
    //     printf(" enter # of %d: ", j);
    //     scanf("%d", &num[j]);
    // }
    DiffCover dc(num_elem, density, threshold);
    dc.Gen0();
    printf("Total = %d\n", dc.total);
}
