"""Plot the current AoS/SoA layout experiment without changing its CSV files."""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

import matplotlib


BENCHMARK_DIR = Path(__file__).resolve().parent.parent
EXPERIMENT_DIR = BENCHMARK_DIR / "layout_experiment"


def select_results(result_dir, requested_timestamp=None):
    """Pair timestamped files; use legacy filenames only if none exist."""
    runs = {}
    for layout in ("aos", "soa"):
        pattern = re.compile(
            rf"{layout}_benchmark(?:_(\d{{8}}_\d{{6}})\.csv|\.csv(\d{{8}}_\d{{6}}))"
        )
        files = {}
        for path in result_dir.glob(f"{layout}_benchmark*"):
            match = pattern.fullmatch(path.name)
            if not path.is_file() or match is None:
                continue
            timestamp = match.group(1) or match.group(2)
            if timestamp in files:
                raise ValueError(f"Multiple {layout.upper()} files for {timestamp}")
            files[timestamp] = path
        runs[layout] = files

    complete = runs["aos"].keys() & runs["soa"].keys()
    available = runs["aos"].keys() | runs["soa"].keys()
    if requested_timestamp:
        if requested_timestamp not in complete:
            raise ValueError(f"No complete AoS/SoA pair for {requested_timestamp}")
        timestamp = requested_timestamp
    elif available:
        if not complete:
            raise ValueError("Timestamped results exist, but no AoS/SoA pair is complete")
        timestamp = max(complete)
        if max(available) > timestamp:
            print(f"Warning: newer incomplete results skipped; using {timestamp}",
                  file=sys.stderr)
    else:
        return (result_dir / "aos_benchmark.csv",
                result_dir / "soa_benchmark.csv", None)

    return runs["aos"][timestamp], runs["soa"][timestamp], timestamp


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
    parser.add_argument("--timestamp", metavar="YYYYMMDD_HHMMSS",
                        help="plot a specific run instead of the latest complete pair")
    parser.add_argument("--show", action="store_true",
                        help="also open an interactive plot window")
    args = parser.parse_args()

    # Save without opening a GUI unless explicitly requested.
    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    result_dir = EXPERIMENT_DIR / "results"
    try:
        aos_path, soa_path, timestamp = select_results(result_dir, args.timestamp)
        aos = read_results(aos_path)
        soa = read_results(soa_path)
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
    title = f"AoS vs SoA | {aos[0][1]} steps per oscillator"
    if timestamp:
        title += f"\nRun: {timestamp}"
    ax.set_title(title)
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=0.25)
    ax.legend(ncols=2)

    suffix = f"_{timestamp}" if timestamp else ""
    figure_path = EXPERIMENT_DIR / "figures" / f"aosvsoa_benchmark{suffix}.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"AoS input: {aos_path}")
    print(f"SoA input: {soa_path}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
