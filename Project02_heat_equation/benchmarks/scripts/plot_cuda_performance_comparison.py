"""Compare CUDA solve wall time with a matching single-thread CPU measurement."""

import argparse
import csv
import math
from pathlib import Path

import matplotlib

from plot_error_experiment import select_result


BENCHMARK_DIR = Path(__file__).resolve().parent.parent
CUDA_EXPERIMENT_DIR = BENCHMARK_DIR / "cuda_experiment"
CUDA_RESULTS_DIR = CUDA_EXPERIMENT_DIR / "results"
CPU_RESULTS_DIR = BENCHMARK_DIR / "parallel_experiment" / "results"
FIGURE_DIR = CUDA_EXPERIMENT_DIR / "figures"
DEFAULT_EXPERIMENT = "cuda_experiment"
CPU_EXPERIMENT = "parallel_experiment"
PARAMETERS = ("nx", "ny", "thermal_diffusivity", "dt", "steps")


def read_results(path, require_baseline=True):
    """Read standard performance CSV rows, keeping CUDA block sizes distinct."""
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source)
        required = {"threads", *PARAMETERS, "ns_per_calu"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        rows = []
        for line, raw in enumerate(reader, start=2):
            if not any((raw.get(name) or "").strip() for name in required):
                continue
            if None in raw or any(raw.get(name) is None for name in required):
                raise ValueError(f"{path}:{line}: expected a complete, comma-separated CSV row")
            row = {name: int(raw[name]) if name in ("threads", "nx", "ny", "steps")
                   else float(raw[name]) for name in required}
            if any(not math.isfinite(value) or value <= 0 for value in row.values()):
                raise ValueError(f"{path}:{line}: parameters and timings must be finite and positive")
            if row["nx"] < 3 or row["ny"] < 3:
                raise ValueError(f"{path}:{line}: grid dimensions must be at least 3")
            rows.append(row)
    if not rows:
        raise ValueError(f"{path}: no measurements")
    if len({row["threads"] for row in rows}) != len(rows):
        raise ValueError(f"{path}: duplicate thread counts or block sizes")
    for name in PARAMETERS:
        if len({row[name] for row in rows}) != 1:
            raise ValueError(f"{path}: {name} must stay fixed within the run")
    if require_baseline and not any(row["threads"] == 1 for row in rows):
        raise ValueError(f"{path}: no single-thread CPU measurement")
    return sorted(rows, key=lambda row: row["threads"])


def validate_comparison(cuda, cpu):
    """Use the measured 1-thread CPU row only, with exactly matched workload."""
    baseline = [row for row in cpu if row["threads"] == 1]
    if len(baseline) != 1:
        raise ValueError("Expected exactly one single-thread CPU measurement")
    baseline = baseline[0]
    for row in cuda:
        for name in PARAMETERS:
            if row[name] != baseline[name]:
                raise ValueError(f"CPU/CUDA workload mismatch: {name}="
                                 f"{baseline[name]} on CPU, {row[name]} on CUDA")
        if row["threads"] > 1024:
            raise ValueError("CUDA threads per block must not exceed 1024")
        ratio = baseline["ns_per_calu"] / row["ns_per_calu"]
        if not math.isfinite(ratio) or ratio <= 0:
            raise ValueError("Timings produce invalid speedup")
    return baseline


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--experiment", default=DEFAULT_EXPERIMENT)
    parser.add_argument("--cpu-experiment", default=CPU_EXPERIMENT)
    parser.add_argument("--timestamp", metavar="YYYYMMDD_HHMMSS")
    parser.add_argument("--cpu-timestamp", metavar="YYYYMMDD_HHMMSS")
    parser.add_argument("--show", action="store_true")
    args = parser.parse_args()
    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    try:
        # Current snapshots are flat; create_result_csv writes future runs in results/.
        cuda_result_dir = CUDA_RESULTS_DIR if CUDA_RESULTS_DIR.is_dir() else CUDA_EXPERIMENT_DIR
        cuda_path, cuda_stamp = select_result(cuda_result_dir, args.experiment,
                                              args.timestamp)
        cpu_path, cpu_stamp = select_result(CPU_RESULTS_DIR, args.cpu_experiment,
                                            args.cpu_timestamp)
        cuda = read_results(cuda_path, require_baseline=False)
        cpu = read_results(cpu_path)
        baseline = validate_comparison(cuda, cpu)
    except (OSError, ValueError, TypeError, OverflowError) as error:
        parser.error(str(error))

    blocks = [row["threads"] for row in cuda]
    timings = [row["ns_per_calu"] for row in cuda]
    cpu_time = baseline["ns_per_calu"]
    speedups = [cpu_time / value for value in timings]
    fig, (time_ax, speedup_ax) = plt.subplots(1, 2, figsize=(11, 5.5), layout="constrained")
    time_ax.plot(blocks, timings, color="#d97706", marker="o", markersize=4,
                 linewidth=1.8, label="CUDA solve call")
    time_ax.axhline(cpu_time, color="#2563eb", linestyle="--", linewidth=1.5,
                    label="CPU single thread")
    time_ax.set_yscale("log")
    time_ax.set_ylabel("Solve wall time / (nx × ny × steps) (ns)")
    time_ax.set_title("Measured solve-call cost (log scale)")
    time_ax.legend()

    speedup_ax.plot(blocks, speedups, color="#d97706", marker="o", markersize=4,
                    linewidth=1.8, label="CPU time / CUDA time")
    speedup_ax.axhline(1, color="#2563eb", linestyle="--", linewidth=1.5,
                       label="Equal performance")
    speedup_ax.set_yscale("log")
    speedup_ax.set_ylabel("Speedup relative to single-thread CPU")
    speedup_ax.set_title("Above 1: CUDA faster; below 1: CPU faster")
    speedup_ax.legend()
    for ax in (time_ax, speedup_ax):
        ax.set_xscale("log", base=2)
        ax.set_xticks(blocks, labels=[str(count) for count in blocks])
        ax.set_xlim(min(blocks) / 1.2, max(blocks) * 1.2)
        ax.set_xlabel("CUDA threads per block")
        ax.grid(True, alpha=0.25)

    fig.suptitle("CUDA vs single-thread CPU: solve-call performance"
                 f"\ngrid={baseline['nx']} × {baseline['ny']} | steps={baseline['steps']}"
                 f" | dt={baseline['dt']:g} | alpha={baseline['thermal_diffusivity']:g}"
                 f"\nCPU run: {cpu_stamp or 'undated'} | CUDA run: {cuda_stamp or 'undated'}",
                 fontsize=11)
    fig.supxlabel("CUDA call includes allocations, H2D/D2H copies, synchronization and frees; "
                  "not kernel-only timing.", fontsize=8)
    figure_path = FIGURE_DIR / (
        f"{args.experiment}_vs_cpu_{cuda_stamp or 'undated'}_cpu_{cpu_stamp or 'undated'}.png")
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"CUDA input: {cuda_path}")
    print(f"CPU input: {cpu_path}; selected threads=1 only")
    print(f"Single-thread CPU: {cpu_time:g} ns per grid-node step")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
