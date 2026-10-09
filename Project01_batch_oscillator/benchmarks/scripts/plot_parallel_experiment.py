"""Plot single-thread and parallel SoA timings versus oscillator count."""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

import matplotlib


BENCHMARK_DIR = Path(__file__).resolve().parent.parent
EXPERIMENT_DIR = BENCHMARK_DIR / "parallel_experiment"
VARIANTS = ("soa", "soa_parallel")


def select_results(result_dir, requested_timestamp=None):
    """Pair timestamped files; use legacy filenames only if none exist."""
    runs = {}
    for variant in VARIANTS:
        pattern = re.compile(
            rf"{variant}_benchmark(?:_(\d{{8}}_\d{{6}})\.csv|\.csv(\d{{8}}_\d{{6}}))"
        )
        files = {}
        for path in result_dir.glob(f"{variant}_benchmark*"):
            match = pattern.fullmatch(path.name)
            if not path.is_file() or match is None:
                continue
            timestamp = match.group(1) or match.group(2)
            if timestamp in files:
                raise ValueError(f"Multiple {variant} files for {timestamp}")
            files[timestamp] = path
        runs[variant] = files

    complete = runs[VARIANTS[0]].keys() & runs[VARIANTS[1]].keys()
    available = runs[VARIANTS[0]].keys() | runs[VARIANTS[1]].keys()
    if requested_timestamp:
        if requested_timestamp not in complete:
            raise ValueError(f"No complete single-thread/parallel pair for {requested_timestamp}")
        timestamp = requested_timestamp
    elif available:
        if not complete:
            raise ValueError("Timestamped results exist, but no single-thread/parallel pair is complete")
        timestamp = max(complete)
        if max(available) > timestamp:
            print(f"Warning: newer incomplete results skipped; using {timestamp}",
                  file=sys.stderr)
    else:
        return (result_dir / f"{VARIANTS[0]}_benchmark.csv",
                result_dir / f"{VARIANTS[1]}_benchmark.csv", None)

    return runs[VARIANTS[0]][timestamp], runs[VARIANTS[1]][timestamp], timestamp


def read_results(path, default_threads=None):
    """Read (N, steps, mean_ns, median_ns, threads), keeping old CSV support."""
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source)
        required = {"N", "steps", "average_ns", "median_ns"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        rows = [
            (int(row["N"]), int(row["steps"]),
             float(row["average_ns"]), float(row["median_ns"]),
             int(row["threads"]) if "threads" in reader.fieldnames else default_threads)
            for row in reader
        ]

    if not rows:
        raise ValueError(f"{path}: no measurements")
    if any(n <= 0 or steps <= 0 or (threads is not None and threads <= 0) or
           not all(math.isfinite(t) and t > 0 for t in (mean, median))
           for n, steps, mean, median, threads in rows):
        raise ValueError(f"{path}: counts, thread counts and timings must be finite and positive")
    if len({row[0] for row in rows}) != len(rows):
        raise ValueError(f"{path}: expected one measurement summary per N")
    if len({row[4] for row in rows}) != 1:
        raise ValueError(f"{path}: expected the same threads for every N")
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
        single_path, parallel_path, timestamp = select_results(result_dir, args.timestamp)
        single = read_results(single_path, default_threads=1)
        parallel = read_results(parallel_path)
        if single[0][4] != 1:
            raise ValueError("Single-thread SoA results must record threads = 1")
        if [row[:2] for row in single] != [row[:2] for row in parallel]:
            raise ValueError("Single-thread and parallel SoA must have matching N and steps")
        if len({row[1] for row in single}) != 1:
            raise ValueError("This parallel plot expects the same steps for every N")
    except (OSError, ValueError, TypeError) as error:
        parser.error(str(error))

    fig, ax = plt.subplots(figsize=(9, 5.5), layout="constrained")
    # Historical parallel CSVs without threads keep a generic label.
    parallel_threads = parallel[0][4]
    parallel_name = "SoA parallel"
    if parallel_threads is not None:
        unit = "thread" if parallel_threads == 1 else "threads"
        parallel_name += f" ({parallel_threads} {unit})"
    for name, rows, color in (("SoA single thread", single, "#2563eb"),
                              (parallel_name, parallel, "#d97706")):
        counts = [row[0] for row in rows]
        ax.plot(counts, [row[2] for row in rows], color=color, marker="o",
                markersize=4, linewidth=1.8, label=f"{name} mean")
        ax.plot(counts, [row[3] for row in rows], color=color, marker="x",
                markersize=4, linestyle="--", linewidth=1.2,
                label=f"{name} median")

    ax.set_xscale("log", base=2)
    ax.set_xlabel("Oscillator count N")
    ax.set_ylabel("Time per oscillator step (ns; lower is faster)")
    title = f"SoA single thread vs parallel | {single[0][1]} steps per oscillator"
    if timestamp:
        title += f"\nRun: {timestamp}"
    ax.set_title(title)
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=0.25)
    ax.legend(ncols=2)

    suffix = f"_{timestamp}" if timestamp else ""
    figure_path = EXPERIMENT_DIR / "figures" / f"parallel_benchmark{suffix}.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"Single-thread SoA input: {single_path}")
    print(f"Parallel SoA input: {parallel_path}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
