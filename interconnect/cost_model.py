#!/usr/bin/env python3
r"""First-order electrical model reported in Table IV of interconnect.md.

Analytic only: illustrative parameters, no layout or SPICE.  Run

    python interconnect/cost_model.py

The model is the Elmore delay of a rail driven at one end,

    t = R_d C_w + 0.5 R_w C_w + L (R_d + 0.5 R_w + R_s) C_s,

with C_w = c_w W and R_w = r_w W, and the switched energy is proportional to
C_w + L C_s.  Only the L-dependent part changes between the dense and the
cyclic design, so the ratios are functions of the single dimensionless group
beta = (R_d + 0.5 R_w + R_s) C_s / (R_d C_w + 0.5 R_w C_w).
"""
from __future__ import annotations

C_W = 0.20      # fF/um
R_W = 0.20      # ohm/um
C_S = 1.0       # fF per switch
R_S = 2.0e3     # ohm per switch
R_D = 5.0e3     # ohm, driver
W = 100.0       # um, rail length
VDD = 0.7       # V

CASES = [
    ("13x13", 13, 13, (0, 1, 3, 9)),
    ("41x12", 41, 12, (0, 1, 4, 6)),
    ("100x23", 100, 23, (0, 1, 2, 3, 7, 11)),
]


def loads(n: int, m: int, d) -> list:
    c = [0] * m
    for i in range(n):
        c[i % m] += 1
    return [sum(c[(j - x) % m] for x in d) for j in range(m)]


def rail(taps: float, w: float = W):
    cw, rw = C_W * w, R_W * w
    a = R_D * cw + 0.5 * rw * cw
    b = (R_D + 0.5 * rw + R_S) * C_S
    return a + b * taps, cw + C_S * taps


def main() -> None:
    print("Table IV: ratios to the dense crossbar")
    print(f"{'design':10s} {'taps':>12s} {'switch':>7s} {'cap':>7s} "
          f"{'delay':>7s} {'energy':>7s}")
    for name, n, m, d in CASES:
        lc = sum(loads(n, m, d)) / m
        td, ct = rail(n)
        ts, cs = rail(lc)
        print(f"{name:10s} {n:5d}->{lc:5.1f} {len(d)/m:7.3f} {cs/ct:7.3f} "
              f"{ts/td:7.3f} {cs/ct:7.3f}")

    print("\nsensitivity to rail length, 13x13 core (delay ratio)")
    for w in (50, 100, 200, 400):
        td, _ = rail(13, w)
        ts, _ = rail(4, w)
        print(f"  W={w:3.0f} um: {ts/td:.3f}")

    print("\nloopback accounting at full load (13 routes)")
    for label, hist in (("k=4", {1: 4, 2: 6, 3: 3}), ("k=5", {1: 5, 2: 8})):
        vias = sum(t * h for t, h in hist.items())
        lbs = sum((t - 1) * h for t, h in hist.items())
        print(f"  {label}: {vias} vias, {lbs} loopbacks")
    print("  k=5 pays when 4 + 4 c_L > 13, i.e. c_L > 2.25 switch units")


if __name__ == "__main__":
    main()
