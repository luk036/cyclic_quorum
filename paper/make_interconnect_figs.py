#!/usr/bin/env python3
r"""Generate the fig-iq-*.svg figures for paper/interconnect.md.

Follows the palette of the existing paper figures (salmon #FA8964, purple
#8964FA, grey #cfcfcf).  Text is emitted as paths (matplotlib's default
svg.fonttype), so rsvg-convert never has to find a font.
"""
from __future__ import annotations

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Circle

SALMON = "#FA8964"
PURPLE = "#8964FA"
GREEN = "#7FB069"
GREY = "#cfcfcf"
LIGHT = "#ececec"
INK = "#333333"

plt.rcParams.update(
    {
        "font.family": "Segoe UI",
        "font.size": 8,
        "axes.edgecolor": GREY,
        "axes.labelcolor": INK,
        "xtick.color": INK,
        "ytick.color": INK,
    }
)

D13 = [0, 1, 3, 9]


def grid_vias(N, M, D):
    g = np.zeros((N, M), dtype=int)
    for i in range(N):
        for d in D:
            g[i, (i + d) % M] = 1
    return g


def draw_grid(ax, g, dot=9.0, line=0.6):
    N, M = g.shape
    for j in range(M):
        ax.plot([-0.5, N - 0.5], [j, j], color=LIGHT, lw=line, zorder=1)
    for i in range(N):
        ax.plot([i, i], [-0.5, M - 0.5], color=LIGHT, lw=line, zorder=1)
    xs, ys = np.where(g == 1)
    ax.scatter(xs, ys, s=dot, color=INK, zorder=3, linewidths=0)
    ax.set_xlim(-0.8, N - 0.2)
    ax.set_ylim(M - 0.2, -0.8)  # output 0 on top
    ax.set_aspect("equal")
    for s in ax.spines.values():
        s.set_visible(False)
    ax.tick_params(length=0, labelsize=6)


def fig_core():
    g = grid_vias(13, 13, D13)
    fig, ax = plt.subplots(figsize=(3.5, 3.5))
    draw_grid(ax, g, dot=11)
    col = 0
    js = np.where(g[col] == 1)[0]
    ax.scatter([col] * len(js), js, s=70, facecolor=SALMON,
               edgecolor=INK, linewidths=0.8, zorder=4)
    ax.set_xlabel("input $i$", labelpad=2)
    ax.set_ylabel("output $j$", labelpad=2)
    ax.set_xticks([0, 3, 6, 9, 12])
    ax.set_yticks([0, 3, 6, 9, 12])
    ax.tick_params(axis="x", labeltop=False, labelbottom=True)
    ax.tick_params(axis="y", labelleft=True, labelright=False)
    fig.tight_layout(pad=0.2)
    fig.savefig("fig-iq-core.svg")
    plt.close(fig)


def fig_scale():
    M = 13
    g = grid_vias(39, M, D13)[:38]
    fig, ax = plt.subplots(figsize=(7.0, 2.7))
    draw_grid(ax, g, dot=7)
    ax.set_xlim(-0.8, 39.6)
    for t in (1, 2):
        ax.axvline(t * 13 - 0.5, color=SALMON, ls="--", lw=1.1, zorder=2)
    # the column that truncation removes: draw its vias as ghosts
    ghost = [(38 + d) % M for d in D13]
    ax.axvline(38, color=SALMON, ls=":", lw=1.0, zorder=2)
    ax.scatter([38] * len(ghost), ghost, s=26, facecolors="#ffffff",
               edgecolors=SALMON, linewidths=1.0, zorder=4)
    ax.text(38, -1.35, "col.\n38", ha="center", va="bottom", fontsize=6.5,
            color=SALMON)
    for t, (a, b) in enumerate([(0, 12), (13, 25), (26, 37)]):
        ax.text((a + b) / 2, M + 0.45,
                f"tile {t}" + (" (truncated)" if t == 2 else ""),
                ha="center", va="center", fontsize=7, color=INK)
    ax.set_ylim(M + 1.0, -1.6)
    ax.set_xlabel("input index $i$ (0 to 37)", labelpad=2)
    ax.set_ylabel("output $j$", labelpad=2)
    ax.set_xticks([0, 12, 25, 37])
    ax.set_yticks([0, 4, 8, 12])
    fig.tight_layout(pad=0.2)
    fig.savefig("fig-iq-scale.svg")
    plt.close(fig)


def fig_wave():
    g = grid_vias(41, 12, [0, 1, 4, 6])
    fig, ax = plt.subplots(figsize=(7.0, 2.3))
    draw_grid(ax, g, dot=8)
    # the five columns beyond three full periods of 12
    ax.axvspan(35.5, 40.5, color=SALMON, alpha=0.10, zorder=0)
    ax.axvline(35.5, color=SALMON, ls="--", lw=1.0, zorder=2)
    ax.text(38.0, 13.35, "5-column remainder", ha="center", va="center",
            fontsize=7.5, color=SALMON)
    ax.set_ylim(14.2, -0.8)
    ax.set_xlabel("input index $i$ (0 to 40)", labelpad=2)
    ax.set_ylabel("output $j$", labelpad=2)
    ax.set_xticks([0, 10, 20, 30, 35, 40])
    ax.set_yticks([0, 3, 6, 9, 11])
    fig.tight_layout(pad=0.2)
    fig.savefig("fig-iq-wave.svg")
    plt.close(fig)


def fig_route():
    fig, ax = plt.subplots(figsize=(4.6, 2.1))
    ax.set_xlim(0, 5.6)
    ax.set_ylim(0.15, 3.15)
    ax.set_aspect("equal")
    ax.axis("off")

    xin, xout = 0.9, 4.2
    i, r = (xin, 2.45), (xout, 2.45)
    ip, j = (xin, 0.85), (xout, 0.85)

    for (x, y), lab in [(i, "$i$"), (ip, "$i'$")]:
        ax.add_patch(Circle((x, y), 0.26, facecolor="#ffffff",
                            edgecolor=PURPLE, lw=1.5, zorder=3))
        ax.text(x, y, lab, ha="center", va="center", fontsize=11, color=INK, zorder=4)
        ax.text(x - 0.40, y, "input", ha="right", va="center", fontsize=8, color="#777777")
    for (x, y), lab in [(r, "$r$"), (j, "$j$")]:
        ax.add_patch(Circle((x, y), 0.26, facecolor="#ffffff",
                            edgecolor=GREEN, lw=1.5, zorder=3))
        ax.text(x, y, lab, ha="center", va="center", fontsize=11, color=INK, zorder=4)
        ax.text(x + 0.40, y, "output", ha="left", va="center", fontsize=8, color="#777777")

    ax.add_patch(FancyArrowPatch(i, r, arrowstyle="-|>", mutation_scale=10,
                                 color=SALMON, lw=2.2, zorder=2,
                                 shrinkA=14, shrinkB=14))
    ax.add_patch(FancyArrowPatch(ip, j, arrowstyle="-|>", mutation_scale=10,
                                 color=SALMON, lw=2.2, zorder=2,
                                 shrinkA=14, shrinkB=14))
    ax.add_patch(FancyArrowPatch(r, ip, arrowstyle="-|>", mutation_scale=10,
                                 color="#888888", lw=1.3, ls=(0, (4, 3)),
                                 zorder=2, connectionstyle="arc3,rad=0.25",
                                 shrinkA=15, shrinkB=15))

    ax.text((i[0] + r[0]) / 2, i[1] + 0.22, "via 1", ha="center", va="center",
            fontsize=9.5, color=INK)
    ax.text((ip[0] + j[0]) / 2, ip[1] + 0.22, "via 2", ha="center", va="center",
            fontsize=9.5, color=INK)
    ax.text((i[0] + r[0]) / 2, 1.62, "loopback (no via)", ha="center", va="center",
            fontsize=8.5, color="#555555",
            bbox=dict(boxstyle="round,pad=0.15", fc="#ffffff", ec="none"))
    fig.tight_layout(pad=0.1)
    fig.savefig("fig-iq-route.svg")
    plt.close(fig)


def fig_scaling():
    fig, ax = plt.subplots(figsize=(5.0, 3.0))
    N = np.linspace(10, 100, 200)
    ax.plot(N, 13 * N, color=GREY, lw=1.4, ls="-", label="dense, $M=13$")
    ax.plot(N, 23 * N, color=GREY, lw=1.4, ls="--", label="dense, $M=23$")
    ax.plot(N, 4 * N, color=SALMON, lw=1.8, label="cyclic, $M=13$, $k=4$")
    ax.plot(N, 6 * N, color=PURPLE, lw=1.8, label="cyclic, $M=23$, $k=6$")
    pts = [(13, 52), (38, 152), (41, 164), (100, 600)]
    ax.scatter([p[0] for p in pts], [p[1] for p in pts], s=18, color=INK, zorder=4)
    for x, y, lab, dx, dy, ha in [
        (13, 52, "$13\\times13$", 5, 7, "left"),
        (38, 152, "$38\\times13$", -4, 12, "right"),
        (41, 164, "$41\\times12$", 4, -14, "left"),
        (100, 600, "$100\\times23$", -5, 8, "right"),
    ]:
        ax.annotate(lab, xy=(x, y), xytext=(dx, dy), textcoords="offset points",
                    fontsize=6.5, color=INK, ha=ha)
    ax.set_xlabel("number of inputs $N$")
    ax.set_ylabel("vias $V$")
    ax.set_xlim(0, 102)
    ax.set_ylim(0, 2320)
    ax.legend(frameon=False, fontsize=7, loc="upper left")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.grid(axis="y", color=LIGHT, lw=0.7)
    ax.set_axisbelow(True)
    fig.tight_layout(pad=0.2)
    fig.savefig("fig-iq-scaling.svg")
    plt.close(fig)


def fig_covers():
    """Coverage strips: differences (12/12) vs sums (10/13) for {0,1,3,9}."""
    D = [0, 1, 3, 9]
    M = 13
    diffs = sorted({(a - b) % M for a in D for b in D if a != b})
    sums = sorted({(a + b) % M for a in D for b in D})

    fig, axes = plt.subplots(2, 1, figsize=(7.2, 2.3))
    rows = [
        (axes[0], diffs, "$\\mathcal{D}-\\mathcal{D}$ (sharing)",
         [r for r in range(1, M)]),
        (axes[1], sums, "$\\mathcal{D}+\\mathcal{D}$ (two-via)",
         list(range(M))),
    ]
    for ax, vals, label, universe in rows:
        ax.set_xlim(-4.6, M + 4.4)
        ax.set_ylim(-0.75, 0.75)
        ax.axis("off")
        ax.text(-0.65, 0, label, ha="right", va="center", fontsize=8, color=INK)
        for r in universe:
            hit = r in vals
            face = "#DFF0D8" if hit else "#ffffff"
            edge = GREEN if hit else SALMON
            ax.add_patch(plt.Rectangle((r - 0.4, -0.33), 0.8, 0.66,
                                       facecolor=face, edgecolor=edge,
                                       linewidth=1.0,
                                       linestyle="-" if hit else (0, (2, 2))))
            ax.text(r, 0, str(r), ha="center", va="center", fontsize=8,
                    color=INK if hit else "#999999")
        missing = [r for r in universe if r not in vals]
        note = f"{len(vals)}/{len(universe)}"
        if missing:
            note += "  missing " + ",".join(map(str, missing))
        else:
            note += "  complete"
        ax.text(M - 0.05, 0, note, ha="left", va="center", fontsize=7.5,
                color=INK)
    fig.tight_layout(pad=0.15)
    fig.savefig("fig-iq-covers.svg")


def fig_rout():
    """Routability vs number of signals, by maximum matching (Table III data)."""
    import random

    def max_matching(adj, m):
        match = [-1] * m

        def aug(u, seen):
            for v in adj[u]:
                if not seen[v]:
                    seen[v] = True
                    if match[v] == -1 or aug(match[v], seen):
                        match[v] = u
                        return True
            return False

        size = 0
        for u in range(len(adj)):
            if aug(u, [False] * m):
                size += 1
        return size

    def curve(n, m, d, samples=3000, seed=3):
        rng = random.Random(seed)
        adj_all = [[(i + x) % m for x in d] for i in range(n)]
        out = []
        for c in range(1, m + 1):
            ok = 0
            for _ in range(samples):
                chosen = rng.sample(range(n), c)
                if max_matching([adj_all[i] for i in chosen], m) == c:
                    ok += 1
            out.append(100.0 * ok / samples)
        return out

    cases = [
        (13, 13, D13, "13$\\times$13 core", INK, "o"),
        (39, 13, D13, "39$\\times$13 tiled", SALMON, "s"),
        (41, 12, [0, 1, 4, 6], "41$\\times$12", PURPLE, "^"),
        (100, 23, [0, 1, 2, 3, 7, 11], "100$\\times$23", GREEN, "D"),
    ]
    fig, ax = plt.subplots(figsize=(5.4, 3.0))
    for n, m, d, label, colour, marker in cases:
        ys = curve(n, m, d)
        ax.plot(range(1, m + 1), ys, color=colour, marker=marker, ms=3.2,
                lw=1.4, label=label)
    ax.set_xlabel("signals routed simultaneously", fontsize=9)
    ax.set_ylabel("routable vectors (%)", fontsize=9)
    ax.set_ylim(94, 100.6)
    ax.set_xlim(0, 24)
    ax.set_yticks([94, 96, 98, 100])
    ax.tick_params(labelsize=8)
    ax.grid(axis="y", color=LIGHT, lw=0.7)
    ax.set_axisbelow(True)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.legend(frameon=False, fontsize=7.5, loc="lower left", ncol=2)
    fig.tight_layout(pad=0.2)
    fig.savefig("fig-iq-rout.svg")
    plt.close(fig)


if __name__ == "__main__":
    fig_core()
    fig_scale()
    fig_wave()
    fig_route()
    fig_scaling()
    fig_covers()
    fig_rout()
    print("wrote fig-iq-*.svg")
