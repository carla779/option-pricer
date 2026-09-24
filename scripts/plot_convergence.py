"""Log-log plot of Monte Carlo error vs number of paths, plain and antithetic.

Usage: python scripts/plot_convergence.py results/convergence.csv results/convergence.png
"""

import csv
import sys

import matplotlib

matplotlib.use("Agg")  # write to file, no window
import matplotlib.pyplot as plt


def main(csv_path: str, png_path: str) -> None:
    with open(csv_path) as f:
        rows = list(csv.DictReader(f))

    paths = [int(r["paths"]) for r in rows]
    std_error = [float(r["std_error"]) for r in rows]
    abs_error = [float(r["abs_error"]) for r in rows]
    av_std_error = [float(r["av_std_error"]) for r in rows]

    # Reference line with slope -1/2, anchored at the first standard error.
    reference = [std_error[0] * (paths[0] / n) ** 0.5 for n in paths]

    fig, ax = plt.subplots(figsize=(7, 4.5))
    ax.loglog(paths, abs_error, "o-", label="|price - Black-Scholes| (plain)")
    ax.loglog(paths, std_error, "s-", label="Standard error (plain)")
    ax.loglog(paths, av_std_error, "^-", label="Standard error (antithetic)")
    ax.loglog(paths, reference, "k--", linewidth=1, label=r"$\propto 1/\sqrt{N}$")
    ax.set_xlabel("Number of paths N")
    ax.set_ylabel("Error")
    ax.set_title("Monte Carlo convergence: ATM European call")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(png_path, dpi=150)
    print(f"Wrote {png_path}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
