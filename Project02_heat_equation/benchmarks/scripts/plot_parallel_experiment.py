"""Plot heat-equation thread scaling and speedup relative to the 1-thread run."""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

import matplotlib

from plot_error_experiment import select_result


EXPERIMENT_DIR = Path(__file__).resolve().parent.parent / "parallel_experiment"
DEFAULT_EXPERIMENT = "parallel_experiment"
PARAMETERS = ("nx", "ny", "thermal_diffusivity", "dt", "steps")


def read_results(path):
    """Read complete measurements, accepting the writer's 'threads nx' field."""
    rows = []
    repaired = 0
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.reader(source)
        header = [name.strip() for name in next(reader, [])]
        required = {"threads", *PARAMETERS, "ns_per_calu"}
        if len(header) != len(set(header)) or not required.issubset(header):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        for line, raw in enumerate(reader, start=2):
            if not any(value.strip() for value in raw):
                continue
            if header[:2] == ["threads", "nx"] and len(raw) == len(header) - 1:
                match = re.fullmatch(r"\s*(\d+)\s+(\d+)\s*", raw[0])
                if match:
                    raw = [match.group(1), match.group(2), *raw[1:]]
                    repaired += 1
            if len(raw) != len(header):
                raise ValueError(f"{path}:{line}: expected {len(header)} fields, got {len(raw)}; "
                                 "incomplete historical rows cannot be reconstructed")
            record = dict(zip(header, raw))
            row = {name: int(record[name]) if name in ("threads", "nx", "ny", "steps")
                   else float(record[name]) for name in required}
            if any(not math.isfinite(value) or value <= 0 for value in row.values()):
                raise ValueError(f"{path}:{line}: parameters and timings must be finite and positive")
            if row["nx"] < 3 or row["ny"] < 3:
                raise ValueError(f"{path}:{line}: grid dimensions must be at least 3")
            rows.append(row)

    if not rows:
        raise ValueError(f"{path}: no measurements")
    if len({row["threads"] for row in rows}) != len(rows):
        raise ValueError(f"{path}: expected one measurement per thread count")
    for name in PARAMETERS:
        if len({row[name] for row in rows}) != 1:
            raise ValueError(f"{path}: {name} must stay fixed during the thread scan")
    if not any(row["threads"] == 1 for row in rows):
        raise ValueError(f"{path}: a 1-thread measurement is required for speedup")
    if repaired:
        print(f"Warning: interpreted 'threads nx' as two fields in {repaired} rows; "
              "source CSV was not changed.", file=sys.stderr)
    return sorted(rows, key=lambda row: row["threads"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--experiment", default=DEFAULT_EXPERIMENT, metavar="NAME",
                        help="result filename prefix without timestamp or .csv "
                             "(default: %(default)s)")
    parser.add_argument("--timestamp", metavar="YYYYMMDD_HHMMSS",
                        help="plot a specific run instead of the latest result")
    parser.add_argument("--show", action="store_true",
                        help="also open an interactive plot window")
    args = parser.parse_args()

    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    try:
        result_path, timestamp = select_result(
            EXPERIMENT_DIR / "results", args.experiment, args.timestamp)
        rows = read_results(result_path)
        threads = [row["threads"] for row in rows]
        timings = [row["ns_per_calu"] for row in rows]
        baseline = next(row["ns_per_calu"] for row in rows if row["threads"] == 1)
        speedups = [baseline / value for value in timings]
        if not all(math.isfinite(value) and value > 0 for value in speedups):
            raise ValueError("Timings produce invalid speedup values")
    except (OSError, ValueError, TypeError, OverflowError) as error:
        parser.error(str(error))

    fig, (time_ax, speedup_ax) = plt.subplots(1, 2, figsize=(11, 5), layout="constrained")
    time_ax.plot(threads, timings, color="#2563eb", marker="o", markersize=4,
                 linewidth=1.8, label="Measured time")
    # Current CSV normalization counts all nodes, including fixed boundaries.
    time_ax.set_ylabel("Time / (nx × ny × steps) (ns; lower is faster)")
    time_ax.set_title("Measured cost per grid-node step")
    time_ax.set_ylim(bottom=0)
    time_ax.legend()

    speedup_ax.plot(threads, speedups, color="#d97706", marker="o", markersize=4,
                    linewidth=1.8, label="Measured speedup")
    speedup_ax.plot(threads, threads, color="#64748b", linestyle="--", linewidth=1.2,
                    label="Ideal linear scaling")
    speedup_ax.set_ylabel("Speedup = time(1 thread) / time(p threads)")
    speedup_ax.set_title("Relative to the same parallel solver at 1 thread")
    speedup_ax.set_ylim(bottom=0)
    speedup_ax.legend()

    for ax in (time_ax, speedup_ax):
        ax.set_xlabel("Thread count")
        ax.set_xticks(threads)
        ax.grid(True, alpha=0.25)

    config = rows[0]
    title = f"Heat diffusion: {args.experiment}"
    if timestamp:
        title += f" | {timestamp}"
    title += (f"\ngrid={config['nx']} × {config['ny']} | steps={config['steps']}"
              f" | dt={config['dt']:g} | alpha={config['thermal_diffusivity']:g}")
    fig.suptitle(title, fontsize=12)

    suffix = f"_{timestamp}" if timestamp else ""
    figure_path = EXPERIMENT_DIR / "figures" / f"{args.experiment}{suffix}.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"Input: {result_path}")
    print(f"Measurements: {len(rows)}; speedup baseline: {baseline:g} ns at 1 thread")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
