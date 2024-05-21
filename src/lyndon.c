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

#include <stdio.h>
#include <stdlib.h>
#define MAX 20
#define MAX_N 200

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------
int N, D, type, NECK = 0, LYN = 0;
int a[MAX], b[MAX];
int matrix[MAX_N][MAX_N];
int differences[MAX_N];
int THRESHOLD;
int D_MINUS_1;
int D_TIMES_D_MINUS_1;
int N_MINUS_D;
int N1;

//-------------------------------------------------------------
void PrintD() {
    // /* Determine minimum position for next bit */
    // int next = (D / p) * a[p] + a[D % p];
    // if (next < N)
    //     return;

    // /* Determine last bit */
    // int min = 1;
    // if ((next == N) && (D % p != 0)) {
    //     min = b[D % p] + 1;
    //     p = D;
    // } else if ((next == N) && (D % p == 0))
    //     min = b[p];

    // for (b[D] = min; b[D] < 2; b[D]++) {
    //     if (LYN && (N % a[p] == 0) && (a[p] != N)) {
    //     } else {
    //         // print a
    //         for (auto i = 1; i <= D; i++) {
    //             printf("%d ", a[i]);
    //         }
    //         printf("\n");
    //         exit(0);

    //         // }
    //         // printf("\n");
    //     }

    // print a
    for (auto i = 1; i <= D; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
    exit(0);
}

/*-----------------------------------------------------------*/
// FIXED DENSITY
/*-----------------------------------------------------------*/
void GenD(int t, int p, int tt) {
    if (t >= THRESHOLD) {
        for (auto i = 0; i < N; i++) {
            differences[i] = 0; // clear
        }
        for (auto i = 0; i < t; i++) {
            for (auto j = i + 1; j <= t; j++) {
                differences[matrix[a[i]][a[j]]] = 1;
                differences[matrix[a[j]][a[i]]] = 1;
            }
        }
        int count = 1;
        for (auto i = 1; i < N; i++) {
            if (differences[i] != 0) {
                count += 1;
            }
        }
        if (count - tt < N1) {
            return;
        }
    }

    int t1 = t + 1;
    if (t1 >= D)
        PrintD();
    else {
        int tail = N_MINUS_D + t1;
        int max = a[t1 - p] + a[p];
        int tt = t1 * (t1 + 1);
        if (max <= tail) {
            a[t1] = max;
            b[t1] = b[t1 - p];

            GenD(t1, p, tt);
            if (b[t1] == 0) {
                b[t1] = 1;
                GenD(t1, t1, tt);
            }
            tail = max - 1;
        }
        for (int j = tail; j >= a[t] + 1; j--) {
            a[t1] = j;
            b[t1] = 1;
            GenD(t1, t1, tt);
        }
    }
}

/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
void Init() {
    int i, j;

    for (j = 0; j <= D; j++)
        a[j] = 0;
    a[D] = N;
    a[0] = N; // for convenience

    D_MINUS_1 = D - 1;
    D_TIMES_D_MINUS_1 = D * (D - 1);
    N_MINUS_D = N - D;
    N1 = N - D_TIMES_D_MINUS_1;

    for (i = 1; i <= N; i++) {
        matrix[i][i] = 0;
        for (j = 1; j < i; j++) {
            matrix[i][j] = i - j;
            matrix[j][i] = N - i + j;
        }
    }

    for (j = N - D + 1; j >= (N - 1) / D + 1; j--) {
        a[1] = j;
        b[1] = 1;
        GenD(1, 1, 2);
    }
    printf("No solution is found.\n");
}

//------------------------------------------------------
void usage() { printf("Usage: necklace [n] [density] [threshold]\n"); }
//--------------------------------------------------------------------------------
int main(int argc, char **argv) {
    if (argc < 4) {
        usage();
        return 1;
    }
    sscanf(argv[1], "%d", &N);
    sscanf(argv[2], "%d", &D);
    sscanf(argv[3], "%d", &THRESHOLD);

    if (N > D * (D - 1) + 1) {
        printf("Error: N must be less than D*(D-1)+1\n");
        return 1;
    }

    Init();
    return 0;
}
