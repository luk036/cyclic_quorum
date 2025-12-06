#include <iostream>
#include <array>

struct Cell {
    int next;
    int prev;
};

struct Element {
    int s;
    int v;
};

class NecklaceGenerator {
public:
    NecklaceGenerator(int n, int num2) :
        n(n), total(0), head(2), nb(0) {
        num[1] = n - num2;
        num[2] = num2;

        // Initialize linked list
        for (int j = 3; j >= 0; j--) {
            avail[j] = {j - 1, j + 1};
        }

        // Initialize arrays
        a.fill(2);
        run.fill(0);
    }

    void generate() {
        // Initial setup
        a[1] = 1;
        num[1]--;
        if (num[1] == 0) listRemove(1);
        B[0] = {0, 0};
        updateRunLength(1);

        gen(2, 1, 1, 2, 1, false);
        std::cout << "Total = " << total << "\n";
    }

private:
    // State
    std::array<Cell, 168> avail;
    std::array<Element, 168> B;
    std::array<int, 3> num;
    std::array<int, 168> a;
    std::array<int, 168> run;
    int n;
    int total;
    int head;
    int nb;

    void listRemove(int i) {
        if (i == head) head = avail[i].next;
        avail[avail[i].prev].next = avail[i].next;
        avail[avail[i].next].prev = avail[i].prev;
    }

    void listAdd(int i) {
        avail[avail[i].next].prev = i;
        avail[avail[i].prev].next = i;
        if (avail[i].prev == 3) head = i;
    }

    int listNext(int i) const {
        return avail[i].next;
    }

    void print(int p) const {
        if (n % p != 0) return;
        // for (int j = 1; j <= n; j++) std::cout << a[j] - 1 << " ";
        // std::cout << "\n";
        const_cast<NecklaceGenerator*>(this)->total++;
    }

    void updateRunLength(int v) {
        if (B[nb].s == v) {
            B[nb].v++;
        } else {
            nb++;
            B[nb] = {v, 1};
        }
    }

    void restoreRunLength() {
        if (B[nb].v == 1) {
            nb--;
        } else {
            B[nb].v--;
        }
    }

    int checkRev() const {
        int j = 1;
        while (j <= nb / 2 &&
               B[j].v == B[nb - j + 1].v &&
               B[j].s == B[nb - j + 1].s) {
            j++;
        }
        if (j > nb / 2) return 0;
        if (B[j].s < B[nb - j + 1].s) return 1;
        if (B[j].s > B[nb - j + 1].s) return -1;

        if (B[j].v < B[nb - j + 1].v && B[j + 1].s < B[nb - j + 1].s) return 1;
        if (B[j].v > B[nb - j + 1].v && B[j].s < B[nb - j].s) return 1;
        return -1;
    }

    void gen(int t, int p, int r, int z, int b, bool RS) {
        // Incremental comparison with reversal
        if (t - 1 > (n - r) / 2 + r) {
            if (a[t - 1] > a[n - t + 2 + r]) {
                RS = false;
            } else if (a[t - 1] < a[n - t + 2 + r]) {
                RS = true;
            }
        }

        // Termination condition - only 2's remain
        if (num[2] == n - t + 1) {
            if (num[2] > run[t - p]) p = n;
            if (num[2] > 0 && t != r + 1 && B[b + 1].s == 2 && B[b + 1].v > num[2]) {
                RS = true;
            }
            if (num[2] > 0 && t != r + 1 && (B[b + 1].s != 2 || B[b + 1].v < num[2])) {
                RS = false;
            }
            if (!RS) print(p);
        }
        // Recursive extension
        else if (num[1] != n - t + 1) {
            int j = head;
            while (j >= a[t - p]) {
                run[z] = t - z;
                updateRunLength(j);
                num[j]--;
                if (num[j] == 0) listRemove(j);
                a[t] = j;

                const int z2 = (j != 2) ? t + 1 : z;
                const int p2 = (j != a[t - p]) ? t : p;

                const int c = checkRev();
                if (c == 0) {
                    std::cout << t << "," << j << " <------\n";
                    gen1(t + 1, p2, t, z2, nb, false);
                }
                if (c == 1) {
                    std::cout << t << "," << j << " <------\n";
                    gen1(t + 1, p2, r, z2, b, RS);
                }
                if (num[j] == 0) listAdd(j);
                num[j]++;
                restoreRunLength();
                j = listNext(j);
            }
            a[t] = 2;
        }
    }

    void gen1(int t, int p, int r, int z, int b, bool RS) {
        // Incremental comparison with reversal
        if (t - 1 > (n - r) / 2 + r) {
            if (a[t - 1] > a[n - t + 2 + r]) {
                RS = false;
            } else if (a[t - 1] < a[n - t + 2 + r]) {
                RS = true;
            }
        }

        // Termination condition - only 2's remain
        if (num[2] == n - t + 1) {
            if (num[2] > run[t - p]) p = n;
            if (num[2] > 0 && t != r + 1 && B[b + 1].s == 2 && B[b + 1].v > num[2]) {
                RS = true;
            }
            if (num[2] > 0 && t != r + 1 && (B[b + 1].s != 2 || B[b + 1].v < num[2])) {
                RS = false;
            }
            if (!RS) print(p);
        }
        // Recursive extension
        else if (num[1] != n - t + 1) {
            int j = head;
            while (j >= a[t - p]) {
                run[z] = t - z;
                updateRunLength(j);
                num[j]--;
                if (num[j] == 0) listRemove(j);
                a[t] = j;

                const int z2 = (j != 2) ? t + 1 : z;
                const int p2 = (j != a[t - p]) ? t : p;

                const int c = checkRev();
                if (c == 0) {
                    gen1(t + 1, p2, t, z2, nb, false);
                }
                if (c == 1) {
                    gen1(t + 1, p2, r, z2, b, RS);
                }
                if (num[j] == 0) listAdd(j);
                num[j]++;
                restoreRunLength();
                j = listNext(j);
            }
            a[t] = 2;
        }
    }
};

int main() {
    int n, num2;
    std::cout << "enter n: ";
    std::cin >> n;
    std::cout << "enter # of 2's: ";
    std::cin >> num2;

    NecklaceGenerator generator(n, num2);
    generator.generate();
    return 0;
}