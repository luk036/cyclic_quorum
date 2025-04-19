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
#define MAX 99

//-------------------------------------------------------------
// GLOBAL VARIABLES
//-------------------------------------------------------------
int N, D, NECK = 0, LYN = 0;
int a[MAX], b[MAX];

//-------------------------------------------------------------
void PrintD(int p) {
    int i, j, next, end, min;

    /* Determine minimum position for next bit */
    next = (D / p) * a[p] + a[D % p];
    if (next < N) return;

    /* Determine last bit */
    min = 1;
    if ((next == N) && (D % p != 0)) {
        min = b[D % p] + 1;
        p = D;
    } else if ((next == N) && (D % p == 0))
        min = b[p];
    end = N;
    for (b[D] = min; b[D] < 2; b[D]++) {
        i = 1;
        if (LYN && (N % a[p] == 0) && (a[p] != N)) {
        } else {
            for (j = 1; j <= end; j++) {
                if (a[i] == j) {
                    printf("%d ", b[i]);
                    i++;
                } else
                    printf("0 ");
            }
            printf("\n");
        }
        p = D;
    }
}
/*-----------------------------------------------------------*/
// FIXED DENSITY
/*-----------------------------------------------------------*/
void GenD(int t, int p) {
    int i, j, max, tail;

    if (t >= D - 1)
        PrintD(p);
    else {
        tail = N - (D - t) + 1;
        max = a[t - p + 1] + a[p];
        if (max <= tail) {
            a[t + 1] = max;
            b[t + 1] = b[t - p + 1];

            GenD(t + 1, p);
            for (i = b[t + 1] + 1; i < 2; i++) {
                b[t + 1] = i;
                GenD(t + 1, t + 1);
            }
            tail = max - 1;
        }
        for (j = tail; j >= a[t] + 1; j--) {
            a[t + 1] = j;
            b[t + 1] = 1;
            GenD(t + 1, t + 1);
        }
    }
}
/*------------------------------------------------------------*/
void Init() {
    int i, j;

    for (j = 0; j <= D; j++) a[j] = 0;
    a[D] = N;
    for (j = N - D + 1; j >= (N - 1) / D + 1; j--) {
        a[1] = j;
        b[1] = 1;
        GenD(1, 1);
    }
}
//------------------------------------------------------
void usage() { printf("Usage: necklace [n] [d] (n>=1, d>=2)\n"); }
//--------------------------------------------------------------------------------
int main(int argc, char **argv) {
    int i, j, n_digit, sum;

    if (argc != 3) {
        usage();
        return 1;
    }
    sscanf(argv[1], "%d", &N);

    if (N < 1) {
        usage();
        return 1;
    }

    // NECKLACES                                LYNDON WORDS
    // -------------                            -------------
    // 2.  Fixed-density                        22. Fixed-density

    NECK = 0;
    LYN = 1;

    sscanf(argv[2], "%d", &D);
    if (D < 2) {
        usage();
        return 1;
    }
    Init();
    return 0;
}
