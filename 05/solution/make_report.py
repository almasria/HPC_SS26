#!/usr/bin/env python3
"""
Build the Exercise 5.2 results table + plot from the two benchmark runs.

The ring_allreduce benchmark prints rows like:
    2^16        0.000123456        0.000234567         OK
one file from the 1-node job, one from the 8-nodes job.

Usage:
    python3 make_report.py ONE_NODE_OUTPUT.txt ONE_NODE_PER_PROCESS_OUTPUT.txt

e.g.
    python3 make_report.py job_hpdc-ex5-1node-12345.txt job_hpdc-ex5-8nodes-12346.txt

Produces:
    results_table.csv   - the four-column table
    results_plot.png    - log-log plot of time vs problem size
"""
import re
import sys
import csv

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROW = re.compile(r"2\^(\d+)\s+([0-9.eE+-]+)\s+([0-9.eE+-]+)")


def parse(path):
    """Return {exponent: (ring_time, mpi_time)} from one job output file."""
    out = {}
    with open(path) as f:
        for line in f:
            m = ROW.search(line)
            if m:
                e = int(m.group(1))
                out[e] = (float(m.group(2)), float(m.group(3)))
    if not out:
        sys.exit(f"No benchmark rows found in {path!r} - check the file.")
    return out


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)

    one_node = parse(sys.argv[1])           # all 8 procs on one node
    per_node = parse(sys.argv[2])           # one proc per node (8 nodes)

    exps = sorted(set(one_node) & set(per_node))
    if not exps:
        sys.exit("The two files share no common problem sizes.")

    header = [
        "Problem Size",
        "Ring Allreduce, One Node [s]",
        "MPI Native, One Node [s]",
        "Ring Allreduce, One Node per Process [s]",
        "MPI Native, One Node per Process [s]",
    ]

    rows = []
    for e in exps:
        r1, m1 = one_node[e]
        r8, m8 = per_node[e]
        rows.append([f"2^{e}", r1, m1, r8, m8])

    # ---- CSV table ----
    with open("results_table.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        for row in rows:
            w.writerow([row[0]] + [f"{v:.9f}" for v in row[1:]])

    # ---- console table ----
    print("\n" + "  ".join(f"{h}" for h in header))
    for row in rows:
        print(f"{row[0]:>8}  " + "  ".join(f"{v:.9f}" for v in row[1:]))

    # ---- plot ----
    x = [2 ** e for e in exps]
    series = {
        "Ring, one node":            ([one_node[e][0] for e in exps], "o-",  "#1f77b4"),
        "MPI native, one node":      ([one_node[e][1] for e in exps], "s--", "#1f77b4"),
        "Ring, one node/process":    ([per_node[e][0] for e in exps], "o-",  "#d62728"),
        "MPI native, one node/proc": ([per_node[e][1] for e in exps], "s--", "#d62728"),
    }

    fig, ax = plt.subplots(figsize=(8, 5.5))
    for label, (y, style, color) in series.items():
        ax.plot(x, y, style, color=color, label=label, linewidth=1.8, markersize=6)

    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel("Problem size [floats]")
    ax.set_ylabel("Average execution time [s] (5 runs)")
    ax.set_title("Ring Allreduce vs. MPI native - 8 processes on octane")
    ax.set_xticks(x)
    ax.set_xticklabels([f"$2^{{{e}}}$" for e in exps])
    ax.grid(True, which="both", linewidth=0.4, alpha=0.6)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig("results_plot.png", dpi=150)
    print("\nWrote results_table.csv and results_plot.png")


if __name__ == "__main__":
    main()
