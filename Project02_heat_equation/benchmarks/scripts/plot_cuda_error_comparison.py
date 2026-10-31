"""Compare CPU and CUDA errors against the analytical solution at matching steps."""

import argparse
import math
from pathlib import Path

import matplotlib

from plot_error_experiment import describe_plot, read_results, select_result


EXPERIMENT_DIR = Path(__file__).resolve().parent.parent / "error_experiment"
DEFAULT_EXPERIMENT = "cuda_error_experiment"
CPU_EXPERIMENT = "baseline_experiment"
MATCH_FIELDS = ("nx", "ny", "thermal_diffusivity", "dt", "steps")


def align_results(cuda, cpu):
    """Match complete configurations; allow extra CPU measurements only."""
    def key(row):
        if not set(MATCH_FIELDS).issubset(row):
            raise ValueError("CPU/CUDA error comparison requires the nx,ny CSV format")
        return tuple(row[name] for name in MATCH_FIELDS)

    cpu_by_key = {key(row): row for row in cpu}
    if len(cpu_by_key) != len(cpu):
        raise ValueError("Duplicate CPU configurations")
    if len({key(row) for row in cuda}) != len(cuda):
        raise ValueError("Duplicate CUDA configurations")
    aligned = []
    for row in cuda:
        if key(row) not in cpu_by_key:
            raise ValueError(f"No CPU error measurement matching {dict(zip(MATCH_FIELDS, key(row)))}")
        aligned.append(cpu_by_key[key(row)])
    _, axis, _, _ = describe_plot(cuda)
    if axis != "steps":
        raise ValueError("This comparison expects a step scan with fixed grid, dt and diffusivity")
    pairs = sorted(zip(cuda, aligned), key=lambda pair: pair[0]["steps"])
    return [pair[0] for pair in pairs], [pair[1] for pair in pairs]


def plot_metric(ax, cuda, cpu, metric):
    for name, rows, color, marker, linestyle in (
            ("CPU", cpu, "#2563eb", "x", "-"),
            ("CUDA", cuda, "#d97706", "o", "--")):
        values = [row[metric] if math.isfinite(row[metric]) and row[metric] > 0
                  else math.nan for row in rows]
        ax.plot([row["steps"] for row in rows], values, color=color, marker=marker,
                markerfacecolor="none", markersize=5, linewidth=1.5,
                linestyle=linestyle, label=name)
        for row in rows:
            value = row[metric]
            if math.isfinite(value) and value > 0:
                continue
            height = 0.04 if value == 0 else 0.96
            ax.plot([row["steps"]], [height], marker="^" if value == 0 else "v",
                    color=color, transform=ax.get_xaxis_transform())
            ax.annotate(f"{name}: steps={row['steps']}, {value:g}",
                        xy=(row["steps"], height), xycoords=ax.get_xaxis_transform(),
                        xytext=(-5, 8 if value == 0 else -12), textcoords="offset points",
                        ha="right", va="bottom" if value == 0 else "top", fontsize=8)
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlim(min(row["steps"] for row in cuda) / 1.2,
                max(row["steps"] for row in cuda) * 1.2)
    ax.set_xlabel("Update steps")
    ax.set_ylabel(f"{metric.upper()} vs analytical solution")
    ax.set_title(metric.upper())
    ax.grid(True, alpha=0.25)
    ax.legend()


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
        cuda_path, cuda_stamp = select_result(EXPERIMENT_DIR / "results", args.experiment,
                                              args.timestamp)
        cpu_path, cpu_stamp = select_result(EXPERIMENT_DIR / "results", args.cpu_experiment,
                                            args.cpu_timestamp)
        cuda, _ = read_results(cuda_path)
        cpu, _ = read_results(cpu_path)
        cpu_count = len(cpu)
        cuda, cpu = align_results(cuda, cpu)
    except (OSError, ValueError, TypeError, OverflowError, ZeroDivisionError) as error:
        parser.error(str(error))

    fig, axes = plt.subplots(1, 2, figsize=(11, 5), layout="constrained")
    for ax, metric in zip(axes, ("mae", "rmse")):
        plot_metric(ax, cuda, cpu, metric)
    config = cuda[0]
    fig.suptitle("CPU vs CUDA: error against the analytical solution"
                 f"\ngrid={config['nx']} × {config['ny']} | dt={config['dt']:g}"
                 f" | alpha={config['thermal_diffusivity']:g}"
                 f"\nCPU run: {cpu_stamp or 'undated'} | CUDA run: {cuda_stamp or 'undated'}",
                 fontsize=11)
    fig.supxlabel("Matching configurations only; these are not direct CPU-CUDA field differences.",
                  fontsize=9)
    figure_path = EXPERIMENT_DIR / "figures" / (
        f"{args.experiment}_vs_cpu_{cuda_stamp or 'undated'}_cpu_{cpu_stamp or 'undated'}.png")
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"CUDA input: {cuda_path}")
    print(f"CPU input: {cpu_path}")
    print(f"Matched: {len(cuda)} measurements; extra CPU measurements omitted: {cpu_count - len(cpu)}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
