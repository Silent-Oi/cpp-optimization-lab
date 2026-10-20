"""Plot MAE/RMSE versus steps for the explicit heat-equation stability experiment."""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

import matplotlib


EXPERIMENT_DIR = Path(__file__).resolve().parent.parent / "error_experiment"
PARAMETERS = ("dx", "dy", "thermal_diffusivity", "dt")


def select_result(result_dir, requested_timestamp=None):
    """Find the latest run, including the current extensionless output format."""
    pattern = re.compile(
        r"unstable_dt_experiment(?:_?(\d{8}_\d{6})(?:\.csv)?|\.csv(\d{8}_\d{6}))"
    )
    runs = {}
    for path in result_dir.glob("unstable_dt_experiment*"):
        match = pattern.fullmatch(path.name)
        if not path.is_file() or match is None:
            continue
        timestamp = match.group(1) or match.group(2)
        if timestamp in runs:
            raise ValueError(f"Multiple result files for {timestamp}")
        runs[timestamp] = path
    if requested_timestamp:
        if requested_timestamp not in runs:
            raise ValueError(f"No result file for {requested_timestamp}")
        return runs[requested_timestamp], requested_timestamp
    if runs:
        timestamp = max(runs)
        return runs[timestamp], timestamp
    for name in ("unstable_dt_experiment.csv", "unstable_dt_experiment"):
        path = result_dir / name
        if path.is_file():
            return path, None
    raise ValueError(f"No unstable_dt_experiment results in {result_dir}")


def read_results(path):
    """Read fixed parameters and (steps, mae, rmse); retain non-finite metrics."""
    config = {}
    rows = []
    skipped = 0
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source)
        required = set(PARAMETERS) | {"steps", "mae", "rmse"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        for line, row in enumerate(reader, start=2):
            def cell(name):
                return (row.get(name) or "").strip()

            if not any(cell(name) for name in required):
                continue
            if None in row:
                raise ValueError(f"{path}:{line}: too many CSV fields")
            for name in PARAMETERS:
                if not cell(name):
                    continue
                value = float(cell(name))
                if not math.isfinite(value) or value <= 0:
                    raise ValueError(f"{path}:{line}: {name} must be finite and positive")
                if name in config and config[name] != value:
                    raise ValueError(f"{path}:{line}: {name} changes within the run")
                config[name] = value
            mae, rmse = cell("mae"), cell("rmse")
            if not mae and not rmse and any(cell(name) for name in PARAMETERS):
                # The current writer emits parameters and a placeholder steps=1.
                skipped += 1
                continue
            if not mae or not rmse:
                raise ValueError(f"{path}:{line}: expected both mae and rmse")
            steps = int(cell("steps"))
            errors = (float(mae), float(rmse))
            if steps <= 0 or any(error < 0 for error in errors):
                raise ValueError(f"{path}:{line}: steps must be positive and errors nonnegative")
            rows.append((steps, *errors))

    if set(config) != set(PARAMETERS):
        raise ValueError(f"{path}: missing parameters {sorted(set(PARAMETERS) - config.keys())}")
    if not rows:
        raise ValueError(f"{path}: no error measurements")
    if len({row[0] for row in rows}) != len(rows):
        raise ValueError(f"{path}: expected one measurement per steps")
    return config, sorted(rows), skipped


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--timestamp", metavar="YYYYMMDD_HHMMSS",
                        help="plot a specific run instead of the latest result")
    parser.add_argument("--show", action="store_true",
                        help="also open an interactive plot window")
    args = parser.parse_args()
    if not args.show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    try:
        result_path, timestamp = select_result(EXPERIMENT_DIR / "results", args.timestamp)
        config, rows, skipped = read_results(result_path)
        rate = config["thermal_diffusivity"] * (1 / config["dx"]**2 + 1 / config["dy"]**2)
        dt_limit = 0.5 / rate
        stability_sum = config["dt"] * rate
        if not all(math.isfinite(value) and value > 0
                   for value in (rate, dt_limit, stability_sum)):
            raise ValueError("Parameters produce non-finite or zero stability coefficients")
    except (OSError, ValueError, TypeError, OverflowError, ZeroDivisionError) as error:
        parser.error(str(error))

    steps = [row[0] for row in rows]
    fig, ax = plt.subplots(figsize=(9, 5.5), layout="constrained")
    for index, name, color, marker in ((1, "MAE", "#2563eb", "o"),
                                      (2, "RMSE", "#d97706", "x")):
        # Break lines at zero/non-finite values rather than inventing finite data.
        values = [row[index] if math.isfinite(row[index]) and row[index] > 0
                  else math.nan for row in rows]
        ax.plot(steps, values, color=color, marker=marker, markersize=4,
                linewidth=1.8, label=name)

    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlim(min(steps) / 1.2, max(steps) * 1.2)
    if not any(math.isfinite(value) and value > 0 for row in rows for value in row[1:]):
        ax.set_ylim(1, 10)
        ax.set_yticks([], minor=False)
        ax.set_yticks([], minor=True)
        ax.text(0.5, 0.5, "No positive finite errors to plot", transform=ax.transAxes,
                ha="center", va="center")
    for count, mae, rmse in rows:
        nonfinite = [f"{name}={value:g}" for name, value in (("MAE", mae), ("RMSE", rmse))
                     if not math.isfinite(value)]
        zeros = [f"{name}=0" for name, value in (("MAE", mae), ("RMSE", rmse)) if value == 0]
        for labels, height, offset, alignment, marker in (
                (nonfinite, 0.97, -12, "top", "v"), (zeros, 0.03, 12, "bottom", "^")):
            if not labels:
                continue
            ax.plot([count], [height], marker=marker, color="#b91c1c",
                    transform=ax.get_xaxis_transform())
            message = f"steps={count}: " + ", ".join(labels)
            ax.annotate(message, xy=(count, height), xycoords=ax.get_xaxis_transform(),
                        xytext=(-6, offset), textcoords="offset points", ha="right",
                        va=alignment, fontsize=9, color="#b91c1c",
                        bbox=dict(facecolor="white", alpha=0.85, edgecolor="none"))
            print(f"Log plot edge annotation: {message}", file=sys.stderr)

    ax.set_xlabel("Update steps")
    ax.set_ylabel("Temperature error (log scale)")
    title = "Explicit heat diffusion: error vs steps"
    if timestamp:
        title += f" | {timestamp}"
    title += (f"\ndt={config['dt']:g}, dt limit (from CSV)={dt_limit:.6g}"
              f" | rx+ry={stability_sum:.6g} (limit 0.5)")
    ax.set_title(title, fontsize=11)
    ax.grid(True, alpha=0.25)
    ax.legend()

    suffix = f"_{timestamp}" if timestamp else ""
    figure_path = EXPERIMENT_DIR / "figures" / f"unstable_dt_experiment{suffix}.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"Input: {result_path}")
    print(f"Parameter-only rows skipped: {skipped}; measurements: {len(rows)}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
