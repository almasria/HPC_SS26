#!/usr/bin/env python3
"""
Build the Exercise 5.3 deliverables from the two process-scaling runs.

The ring_allreduce_procscale benchmark prints rows like:
    P=2          0.001234567        0.002345678        OK
one file from the 1-node job, one from the 8-nodes job.

Usage:
    python3 make_report_53.py ONE_NODE_OUTPUT.txt ONE_NODE_PER_PROCESS_OUTPUT.txt

Produces:
    results_53_time.csv      - Table 1 (execution time, 4 columns)
    results_53_speedup.csv   - Table 2 (speedup, 4 columns)
    results_53_plot.png      - two panels: time vs P, speedup vs P

SPEEDUP DEFINITION (assumption -- confirm with your tutor):
    Per column, relative to the smallest process count P=2:
        S(P) = T(2) / T(P)
    There is no single-process baseline because allreduce on 1 process does
    no communication. With fixed per-process data, adding processes adds
    communication, so S(P) is typically < 1 (a slowdown); that is an expected
    and reportable result. If the tutor wants a different baseline, change
    BASELINE_P below.
"""
import re
import sys
import csv

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

BASELINE_P = 2
ROW = re.compile(r"P=\s*(\d+)\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)")


def parse(path):
    out = {}
    with open(path) as f:
        for line in f:
            m = ROW.search(line)
            if m:
                out[int(m.group(1))] = (float(m.group(2)), float(m.group(3)))
    if not out:
        sys.exit(f"No benchmark rows found in {path!r} - check the file.")
    return out


def speedup(col, baseline):
    return [baseline / v if v > 0 else float("nan") for v in col]


def write_csv(path, header, pvals, columns):
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        for i, p in enumerate(pvals):
            w.writerow([p] + [f"{c[i]:.9f}" for c in columns])


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)

    one = parse(sys.argv[1])   # 8 procs on one node
    per = parse(sys.argv[2])   # one proc per node

    P = sorted(set(one) & set(per))
    if BASELINE_P not in P:
        sys.exit(f"Baseline P={BASELINE_P} missing from the data.")

    ring_one = [one[p][0] for p in P]
    mpi_one  = [one[p][1] for p in P]
    ring_per = [per[p][0] for p in P]
    mpi_per  = [per[p][1] for p in P]

    # ---- Table 1: execution time ----
    t_header = [
        "Process Count",
        "Ring Allreduce, One Node [s]",
        "MPI Native, One Node [s]",
        "Ring Allreduce, One Node per Process [s]",
        "MPI Native, One Node per Process [s]",
    ]
    write_csv("results_53_time.csv", t_header, P,
              [ring_one, mpi_one, ring_per, mpi_per])

    # ---- Table 2: speedup (per column, baseline P=BASELINE_P) ----
    bi = P.index(BASELINE_P)
    s_ring_one = speedup(ring_one, ring_one[bi])
    s_mpi_one  = speedup(mpi_one,  mpi_one[bi])
    s_ring_per = speedup(ring_per, ring_per[bi])
    s_mpi_per  = speedup(mpi_per,  mpi_per[bi])
    s_header = [
        "Process Count",
        "Ring Allreduce, One Node",
        "MPI Native, One Node",
        "Ring Allreduce, One Node per Process",
        "MPI Native, One Node per Process",
    ]
    write_csv("results_53_speedup.csv", s_header, P,
              [s_ring_one, s_mpi_one, s_ring_per, s_mpi_per])

    # ---- console preview ----
    print("\n== Table 1: execution time [s] ==")
    print("  P   ring_1node     mpi_1node      ring_perNode   mpi_perNode")
    for i, p in enumerate(P):
        print(f"  {p}  {ring_one[i]:.9f}  {mpi_one[i]:.9f}  "
              f"{ring_per[i]:.9f}  {mpi_per[i]:.9f}")
    print(f"\n== Table 2: speedup (baseline P={BASELINE_P}) ==")
    print("  P   ring_1node  mpi_1node  ring_perNode  mpi_perNode")
    for i, p in enumerate(P):
        print(f"  {p}  {s_ring_one[i]:9.4f}  {s_mpi_one[i]:9.4f}  "
              f"{s_ring_per[i]:9.4f}  {s_mpi_per[i]:9.4f}")

    # ---- plot: two panels ----
    fig, (axt, axs) = plt.subplots(1, 2, figsize=(13, 5.2))

    style = {
        "Ring, one node":           ("o-",  "#1f77b4"),
        "MPI native, one node":     ("s--", "#1f77b4"),
        "Ring, one node/process":   ("o-",  "#d62728"),
        "MPI native, one node/proc":("s--", "#d62728"),
    }
    times = [ring_one, mpi_one, ring_per, mpi_per]
    speeds = [s_ring_one, s_mpi_one, s_ring_per, s_mpi_per]

    for (label, (st, col)), y in zip(style.items(), times):
        axt.plot(P, y, st, color=col, label=label, linewidth=1.8, markersize=6)
    axt.set_yscale("log")
    axt.set_xlabel("Process count P")
    axt.set_ylabel("Average execution time [s] (5 runs)")
    axt.set_title("Execution time vs. process count (N = 1,000,000)")
    axt.set_xticks(P)
    axt.grid(True, which="both", linewidth=0.4, alpha=0.6)
    axt.legend(frameon=False, fontsize=9)

    for (label, (st, col)), y in zip(style.items(), speeds):
        axs.plot(P, y, st, color=col, label=label, linewidth=1.8, markersize=6)
    axs.axhline(1.0, color="gray", linewidth=0.8, linestyle=":")
    axs.set_xlabel("Process count P")
    axs.set_ylabel(f"Speedup  S(P) = T({BASELINE_P}) / T(P)")
    axs.set_title("Speedup vs. process count")
    axs.set_xticks(P)
    axs.grid(True, which="both", linewidth=0.4, alpha=0.6)
    axs.legend(frameon=False, fontsize=9)

    fig.tight_layout()
    fig.savefig("results_53_plot.png", dpi=150)
    print("\nWrote results_53_time.csv, results_53_speedup.csv, results_53_plot.png")


if __name__ == "__main__":
    main()
