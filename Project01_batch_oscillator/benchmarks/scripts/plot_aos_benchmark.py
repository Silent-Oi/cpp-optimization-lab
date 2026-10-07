"""Plot the current AoS/SoA layout experiment without changing its CSV files."""

import argparse
import csv
import math
from pathlib import Path

import matplotlib


BENCHMARK_DIR = Path(__file__).resolve().parent.parent
EXPERIMENT_DIR = BENCHMARK_DIR / "layout_experiment"


def read_results(path):
    """Read (N, steps, mean_ns, median_ns), ordered by oscillator count."""
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source)
        required = {"N", "steps", "average_ns", "median_ns"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        rows = [
            (int(row["N"]), int(row["steps"]),
             float(row["average_ns"]), float(row["median_ns"]))
            for row in reader
        ]

    if not rows:
        raise ValueError(f"{path}: no measurements")
    if any(n <= 0 or steps <= 0 or
           not all(math.isfinite(t) and t > 0 for t in (mean, median))
           for n, steps, mean, median in rows):
        raise ValueError(f"{path}: counts and timings must be finite and positive")
    if len({row[0] for row in rows}) != len(rows):
        raise ValueError(f"{path}: expected one measurement summary per N")
    return sorted(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--show", action="store_true",
                        help="also open an interactive plot window")
    args = parser.parse_args()

    # Save without opening a GUI unless explicitly requested.
    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    result_dir = EXPERIMENT_DIR / "results"
    try:
        aos = read_results(result_dir / "aos_benchmark.csv")
        soa = read_results(result_dir / "soa_benchmark.csv")
        if [row[:2] for row in aos] != [row[:2] for row in soa]:
            raise ValueError("AoS and SoA must have matching N and steps")
        if len({row[1] for row in aos}) != 1:
            raise ValueError("This layout plot expects the same steps for every N")
    except (OSError, ValueError, TypeError) as error:
        parser.error(str(error))

    fig, ax = plt.subplots(figsize=(9, 5.5), layout="constrained")
    for name, rows, color in (("AoS", aos, "#2563eb"),
                              ("SoA", soa, "#d97706")):
        counts = [row[0] for row in rows]
        ax.plot(counts, [row[2] for row in rows], color=color, marker="o",
                markersize=4, linewidth=1.8, label=f"{name} mean")
        ax.plot(counts, [row[3] for row in rows], color=color, marker="x",
                markersize=4, linestyle="--", linewidth=1.2,
                label=f"{name} median")

    ax.set_xscale("log", base=2)
    ax.set_xlabel("Oscillator count N")
    ax.set_ylabel("Time per oscillator step (ns; lower is faster)")
    ax.set_title(f"AoS vs SoA | {aos[0][1]} steps per oscillator")
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=0.25)
    ax.legend(ncols=2)

    figure_path = EXPERIMENT_DIR / "figures" / "aosvsoa_benchmark.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"Input: {result_dir}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
