#include <stdio.h>

#define TRUE 1
#define FALSE 0
typedef struct cell {
    int next, prev;
} cell;
typedef struct element {
    int s, v;
} element;
cell avail[50];
element B[50];
int nb = 0;
// run length encoding data structure
// number of blocks
int num[50], a[50], run[50], n, k, total, head, NECK = 1, LYN = 0;
/*-----------------------------------------------------------*/
void ListRemove(int i) {
    int p, n;
    if (i == head) head = avail[i].next;
    p = avail[i].prev;
    n = avail[i].next;
    avail[p].next = n;
    avail[n].prev = p;
}
void ListAdd(int i) {
    int p, n;
    p = avail[i].prev;
    n = avail[i].next;
    avail[n].prev = i;
    avail[p].next = i;
    if (avail[i].prev == k + 1) head = i;
}
int ListNext(int i) { return avail[i].next; }
/*-----------------------------------------------------------*/
void Print(int p) {
    int j;
    if (NECK && n % p != 0) return;
    if (LYN && n != p) return;
    for (j = 1; j <= n; j++) printf("%d ", a[j] - 1);
    printf("\n");
    total++;
}
/*-----------------------------------------------------------*/
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
/*---------------------------------------------------------------------*/
// return-1 if reverse smaller, 0 if equal, and 1 if reverse is larger
/*---------------------------------------------------------------------*/
int CheckRev() {
    int j;
    j = 1;
    while (B[j].v == B[nb - j + 1].v && B[j].s == B[nb - j + 1].s && j <= nb / 2) j++;
    if (j > nb / 2) return 0;
    if (B[j].s < B[nb - j + 1].s) return 1;
    if (B[j].s > B[nb - j + 1].s) return -1;

    if (B[j].v < B[nb - j + 1].v && B[j + 1].s < B[nb - j + 1].s) return 1;
    if (B[j].v > B[nb - j + 1].v && B[j].s < B[nb - j].s) return 1;
    return -1;
}
/*-----------------------------------------------------------*/
void Gen(int t, int p, int r, int z, int b, int RS) {
    int j, z2, p2, c;
    // Incremental comparison of a[r+1...n] with its reversal
    if (t - 1 > (n - r) / 2 + r) {
        if (a[t - 1] > a[n - t + 2 + r])
            RS = FALSE;
        else if (a[t - 1] < a[n - t + 2 + r])
            RS = TRUE;
    }
    // Termination condition- only characters k remain to be appended
    if (num[k] == n - t + 1) {
        if (num[k] > run[t - p]) p = n;
        if (num[k] > 0 && t != r + 1 && B[b + 1].s == k && B[b + 1].v > num[k]) RS = TRUE;
        if (num[k] > 0 && t != r + 1 && (B[b + 1].s != k || B[b + 1].v < num[k])) RS = FALSE;
        if (RS == FALSE) Print(p);
    }
    // Recursively extend the prenecklace- unless only 0s remain to be appended
    else if (num[1] != n - t + 1) {
        j = head;
        while (j >= a[t - p]) {
            run[z] = t - z;
            UpdateRunLength(j);
            num[j]--;
            if (num[j] == 0) ListRemove(j);
            a[t] = j;
            z2 = z;
            if (j != k) z2 = t + 1;
            p2 = p;
            if (j != a[t - p]) p2 = t;
            c = CheckRev();
            if (c == 0) Gen(t + 1, p2, t, z2, nb, FALSE);
            if (c == 1) Gen(t + 1, p2, r, z2, b, RS);
            if (num[j] == 0) ListAdd(j);
            num[j]++;
            RestoreRunLength();
            j = ListNext(j);
        }
        a[t] = k;
    }
}
/*-----------------------------------------------------------*/
int main() {
    int j;
    printf("enter n k: ");
    scanf("%d %d", &n, &k);
    for (j = 1; j <= k; j++) {
        printf(" enter # of %d’s: ", j);
        scanf("%d", &num[j]);
    }
    for (j = k + 1; j >= 0; j--) {
        avail[j].next = j - 1;
        avail[j].prev = j + 1;
    }
    head = k;
    for (j = 1; j <= n; j++) {
        a[j] = k;
        run[j] = 0;
    }
    total = 0;
    a[1] = 1;
    num[1]--;
    if (num[1] == 0) ListRemove(1);
    B[0].s = 0;
    UpdateRunLength(1);
    Gen(2, 1, 1, 2, 1, FALSE);
    printf("Total = %d\n", total);
}
