#include <stdio.h>

typedef struct cell {
    int next, prev;
} cell;

typedef struct element {
    int s, v;
} element;

typedef struct {
    // Constants
    const int TRUE;
    const int FALSE;

    // Run-length encoding data
    element B[50];
    int nb;

    // Necklace generation state
    cell avail[50];
    int num[3];  // Only need counts for 1 and 2 (0 is unused)
    int a[50];
    int run[50];
    int n;
    int total;
    int head;
    int NECK;
    int LYN;
} ProgramState;

void ListRemove(ProgramState* state, int i) {
    if (i == state->head) state->head = state->avail[i].next;
    int p = state->avail[i].prev;
    int n = state->avail[i].next;
    state->avail[p].next = n;
    state->avail[n].prev = p;
}

void ListAdd(ProgramState* state, int i) {
    int p = state->avail[i].prev;
    int n = state->avail[i].next;
    state->avail[n].prev = i;
    state->avail[p].next = i;
    if (state->avail[i].prev == 3) state->head = i;
}

int ListNext(ProgramState* state, int i) { return state->avail[i].next; }

void Print(ProgramState* state, int p) {
    if (state->NECK && state->n % p != 0) return;
    if (state->LYN && state->n != p) return;
    for (int j = 1; j <= state->n; j++) printf("%d ", state->a[j] - 1);
    printf("\n");
    state->total++;
}

void UpdateRunLength(ProgramState* state, int v) {
    if (state->B[state->nb].s == v) {
        state->B[state->nb].v++;
    } else {
        state->nb++;
        state->B[state->nb].v = 1;
        state->B[state->nb].s = v;
    }
}

void RestoreRunLength(ProgramState* state) {
    if (state->B[state->nb].v == 1) {
        state->nb--;
    } else {
        state->B[state->nb].v--;
    }
}

int CheckRev(ProgramState* state) {
    int j = 1;
    while (j <= state->nb / 2 && state->B[j].v == state->B[state->nb - j + 1].v
           && state->B[j].s == state->B[state->nb - j + 1].s) {
        j++;
    }
    if (j > state->nb / 2) return 0;
    if (state->B[j].s < state->B[state->nb - j + 1].s) return 1;
    if (state->B[j].s > state->B[state->nb - j + 1].s) return -1;

    if (state->B[j].v < state->B[state->nb - j + 1].v
        && state->B[j + 1].s < state->B[state->nb - j + 1].s)
        return 1;
    if (state->B[j].v > state->B[state->nb - j + 1].v && state->B[j].s < state->B[state->nb - j].s)
        return 1;
    return -1;
}

void Gen(ProgramState* state, int t, int p, int r, int z, int b, int RS) {
    // Incremental comparison with reversal
    if (t - 1 > (state->n - r) / 2 + r) {
        if (state->a[t - 1] > state->a[state->n - t + 2 + r]) {
            RS = state->FALSE;
        } else if (state->a[t - 1] < state->a[state->n - t + 2 + r]) {
            RS = state->TRUE;
        }
    }

    // Termination condition - only 2's remain
    if (state->num[2] == state->n - t + 1) {
        if (state->num[2] > state->run[t - p]) p = state->n;
        if (state->num[2] > 0 && t != r + 1 && state->B[b + 1].s == 2
            && state->B[b + 1].v > state->num[2]) {
            RS = state->TRUE;
        }
        if (state->num[2] > 0 && t != r + 1
            && (state->B[b + 1].s != 2 || state->B[b + 1].v < state->num[2])) {
            RS = state->FALSE;
        }
        if (RS == state->FALSE) Print(state, p);
    }
    // Recursive extension
    else if (state->num[1] != state->n - t + 1) {
        int j = state->head;
        while (j >= state->a[t - p]) {
            state->run[z] = t - z;
            UpdateRunLength(state, j);
            state->num[j]--;
            if (state->num[j] == 0) ListRemove(state, j);
            state->a[t] = j;

            int z2 = (j != 2) ? t + 1 : z;
            int p2 = (j != state->a[t - p]) ? t : p;

            int c = CheckRev(state);
            if (c == 0) Gen(state, t + 1, p2, t, z2, state->nb, state->FALSE);
            if (c == 1) Gen(state, t + 1, p2, r, z2, b, RS);

            if (state->num[j] == 0) ListAdd(state, j);
            state->num[j]++;
            RestoreRunLength(state);
            j = ListNext(state, j);
        }
        state->a[t] = 2;
    }
}

int main() {
    ProgramState state
        = {.TRUE = 1, .FALSE = 0, .nb = 0, .n = 0, .total = 0, .head = 2, .NECK = 1, .LYN = 0};

    printf("enter n: ");
    scanf("%d", &state.n);
    printf(" enter # of 1's: ");
    scanf("%d", &state.num[1]);
    printf(" enter # of 2's: ");
    scanf("%d", &state.num[2]);

    // Initialize linked list
    for (int j = 3; j >= 0; j--) {
        state.avail[j].next = j - 1;
        state.avail[j].prev = j + 1;
    }

    // Initialize arrays
    for (int j = 1; j <= state.n; j++) {
        state.a[j] = 2;
        state.run[j] = 0;
    }

    // Initial setup
    state.a[1] = 1;
    state.num[1]--;
    if (state.num[1] == 0) ListRemove(&state, 1);
    state.B[0].s = 0;
    UpdateRunLength(&state, 1);

    Gen(&state, 2, 1, 1, 2, 1, state.FALSE);
    printf("Total = %d\n", state.total);
    return 0;
}
