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
 
 typedef struct cell {
     int next, prev;
 } cell;
 
 typedef struct element {
     int s, v;
 } element;
 
 class NecklaceGenerator {
 private:
     // Member variables (formerly global)
     int N, K, D, M, type, head, NECK, LYN;
     long long int total, limit;
     int a[MAX], b[MAX], num[MAX], run[MAX], num_map[MAX];
     cell avail[MAX];
     int nb;              // number of blocks
     element B[MAX];      // run length encoding data structure
 
 public:
     NecklaceGenerator() : NECK(0), LYN(0), total(0), limit(-1), nb(0) {}
 
     void Print() {
         int j;
 
         if (limit >= 0 && total >= limit) {
             printf("output limit reached\n");
             exit(0);
         }
 
         total++;
         for (j = 1; j <= N; j++) {
             printf("%d ", num_map[a[j]] - 1);
         }
         printf("\n");
     }
 
     void Remove(int i) {
         int p, n;
 
         if (i == head)
             head = avail[i].next;
         p = avail[i].prev;
         n = avail[i].next;
         avail[p].next = n;
         avail[n].prev = p;
     }
 
     void Add(int i) {
         int p, n;
 
         p = avail[i].prev;
         n = avail[i].next;
         avail[n].prev = i;
         avail[p].next = i;
         if (avail[i].prev == 3)
             head = i;
     }
 
     int CheckRev(int t, int i) {
         int j;
 
         for (j = i + 1; j <= (t + 1) / 2; j++) {
             if (a[j] < a[t - j + 1])
                 return (0);
             else if (a[j] > a[t - j + 1])
                 return (-1);
         }
         return (1);
     }
 
     int CheckRevF() {
         int j;
 
         j = 1;
         while (B[j].v == B[nb - j + 1].v && B[j].s == B[nb - j + 1].s &&
                j <= nb / 2)
             j++;
         if (j > nb / 2)
             return 0;
         if (B[j].s < B[nb - j + 1].s)
             return 1;
         if (B[j].s > B[nb - j + 1].s)
             return -1;
         if (B[j].v < B[nb - j + 1].v && B[j + 1].s < B[nb - j + 1].s)
             return 1;
         if (B[j].v > B[nb - j + 1].v && B[j].s < B[nb - j].s)
             return 1;
         return -1;
     }
 
     void UpdateRunLength(int v) {
         if (B[nb].s == v)
             B[nb].v = B[nb].v + 1;
         else {
             nb++;
             B[nb].v = 1;
             B[nb].s = v;
         }
     }
 
     void RestoreRunLength() {
         if (B[nb].v == 1)
             nb--;
         else
             B[nb].v = B[nb].v - 1;
     }
 
     void GenBF(int t, int p, int r, int z, int b, bool RS) {
         int j, z2, p2, c;
         int i, count, order[20]; // used for lex generation
 
         // Incremental comparison of a[r+1...n] with its reversal
         if (t - 1 > (N - r) / 2 + r) {
             if (a[t - 1] > a[N - t + 2 + r])
                 RS = false;
             else if (a[t - 1] < a[N - t + 2 + r])
                 RS = true;
         }
 
         // Termination condition - only characters k remain to be appended
         if (num[2] == N - t + 1) {
             if (num[2] > run[t - p])
                 p = N;
             if (num[2] > 0 && t != r + 1 && B[b + 1].s == 2 && B[b + 1].v > num[2])
                 RS = true;
             if (num[2] > 0 && t != r + 1 &&
                 (B[b + 1].s != 2 || B[b + 1].v < num[2]))
                 RS = false;
             if ((RS == false) && (N == p))
                 Print();
         }
         // Recursively extend the prenecklace - unless only 0s remain to be appended
         else if (num[1] != N - t + 1) {
             j = head;
 
             // generate in lex order
             i = 0;
             while (j >= a[t - p]) {
                 order[++i] = j;
                 j = avail[j].next;
             }
 
             for (count = i; count >= 1; count--) {
                 j = order[count];
 
                 run[z] = t - z;
                 UpdateRunLength(j);
                 num[j]--;
                 if (num[j] == 0)
                     Remove(j);
                 a[t] = j;
                 z2 = z;
                 if (j != 2)
                     z2 = t + 1;
                 p2 = p;
                 if (j != a[t - p])
                     p2 = t;
                 c = CheckRevF();
                 if (c == 0)
                     GenBF(t + 1, p2, t, z2, nb, false);
                 if (c == 1)
                     GenBF(t + 1, p2, r, z2, b, RS);
                 if (num[j] == 0)
                     Add(j);
                 num[j]++;
                 RestoreRunLength();
             }
             a[t] = 2;
         }
     }
 
     void Init() {
         int i, j;
 
         a[0] = a[1] = 0;
         for (j = 3; j >= 0; j--) {
             avail[j].next = j - 1;
             avail[j].prev = j + 1;
         }
         head = 2;
         for (j = 1; j <= N; j++) {
             a[j] = 2;
             run[j] = 0;
         }
         a[1] = 1;
         num[1]--;
         if (num[1] == 0)
             Remove(1);
         B[0].s = 0;
         UpdateRunLength(1);
         GenBF(2, 1, 1, 2, 1, false);
     }
 
     static void usage() {
         printf("Usage: necklace [type] [n] [k] [max] [d | m forbid_substring | "
                "content] (1<=type<=12 or 21<=type<=27, n>=1, k>=2)\n");
     }
 
     int main(int argc, char **argv) {
         int i, j, n_digit, sum;
 
         if (argc < 5) {
             usage();
             return 1;
         }
         sscanf(argv[1], "%d", &type);
         sscanf(argv[2], "%d", &N);
         sscanf(argv[3], "%d", &K);
         sscanf(argv[4], "%lld", &limit);
 
         if ((type < 1) || ((type > 12) && (type < 21)) || (type > 27) || (N < 1) ||
             (K < 2)) {
             usage();
             return 1;
         }
 
         i = 1;
         sum = 0;
         if (argc != 5 + K) {
             printf("invalid content\n");
             return 1;
         }
         for (j = 1; j <= K; j++) {
             sscanf(argv[4 + j], "%d", &n_digit);
             if (n_digit != 0) {
                 num[i] = n_digit;
                 num_map[i] = j;
                 i++;
                 sum += n_digit;
             }
         }
         if (sum != N) {
             printf("invalid content\n");
             return 1;
         }
         K = i - 1;
         
         Init();
         return 0;
     }
 };
 
 int main(int argc, char **argv) {
     NecklaceGenerator generator;
     return generator.main(argc, argv);
 }
