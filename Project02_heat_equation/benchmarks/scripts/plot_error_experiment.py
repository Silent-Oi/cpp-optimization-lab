"""Plot MAE/RMSE for a named step, grid, or time-step scan."""

import argparse
import csv
import math
import re
import sys
from pathlib import Path

import matplotlib


EXPERIMENT_DIR = Path(__file__).resolve().parent.parent / "error_experiment"
# Change this filename prefix to select the default experiment (no timestamp).
DEFAULT_EXPERIMENT = "time_step_refinement_experiment"
# Physical side lengths for nx/ny CSVs; both endpoints are grid nodes.
DOMAIN_LENGTH_X = 1.0
DOMAIN_LENGTH_Y = 1.0


def select_result(result_dir, experiment, requested_timestamp=None):
    """Find the latest run, including the current extensionless output format."""
    if not experiment or experiment in (".", "..") or any(c in experiment for c in "/\\"):
        raise ValueError("Experiment must be a filename prefix, not a path")
    pattern = re.compile(
        re.escape(experiment) + r"(?:_?(\d{8}_\d{6})(?:\.csv)?|\.csv(\d{8}_\d{6}))"
    )
    runs = {}
    for path in result_dir.iterdir():
        match = pattern.fullmatch(path.name)
        if not path.is_file() or match is None:
            continue
        timestamp = match.group(1) or match.group(2)
        if timestamp in runs:
            raise ValueError(f"Multiple {experiment} result files for {timestamp}")
        runs[timestamp] = path
    if requested_timestamp:
        if requested_timestamp not in runs:
            raise ValueError(f"No {experiment} result file for {requested_timestamp}")
        return runs[requested_timestamp], requested_timestamp
    if runs:
        timestamp = max(runs)
        return runs[timestamp], timestamp
    for name in (f"{experiment}.csv", experiment):
        path = result_dir / name
        if path.is_file():
            return path, None
    raise ValueError(f"No {experiment} results in {result_dir}")


def read_results(path):
    """Carry forward parameter cells; retain measurements as complete rows."""
    state = {}
    rows = []
    skipped = 0
    with path.open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source)
        columns = set(reader.fieldnames or [])
        if {"dx", "dy"}.issubset(columns):
            grid_fields = ("dx", "dy")
        elif {"nx", "ny"}.issubset(columns):
            grid_fields = ("nx", "ny")
        else:
            raise ValueError(f"{path}: expected grid columns dx,dy or nx,ny")
        parameters = (*grid_fields, "thermal_diffusivity", "dt", "steps")
        required = set(parameters) | {"mae", "rmse"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: expected columns {sorted(required)}")
        for line, row in enumerate(reader, start=2):
            def cell(name):
                return (row.get(name) or "").strip()

            if not any(cell(name) for name in required):
                continue
            if None in row:
                raise ValueError(f"{path}:{line}: too many CSV fields")
            for name in parameters:
                if not cell(name):
                    continue
                value = int(cell(name)) if name in ("nx", "ny", "steps") else float(cell(name))
                if not math.isfinite(value) or value <= 0:
                    raise ValueError(f"{path}:{line}: {name} must be finite and positive")
                if name in ("nx", "ny") and value < 3:
                    raise ValueError(f"{path}:{line}: {name} must be at least 3")
                state[name] = value
            mae, rmse = cell("mae"), cell("rmse")
            if not mae and not rmse and any(cell(name) for name in parameters):
                # The writer emits a configuration row without error metrics.
                skipped += 1
                continue
            if not mae or not rmse:
                raise ValueError(f"{path}:{line}: expected both mae and rmse")
            missing = set(parameters) - state.keys()
            if missing:
                raise ValueError(f"{path}:{line}: missing parameters {sorted(missing)}")
            errors = (float(mae), float(rmse))
            if any(error < 0 for error in errors):
                raise ValueError(f"{path}:{line}: errors must be nonnegative")
            rows.append({**state, "mae": errors[0], "rmse": errors[1]})

    if not rows:
        raise ValueError(f"{path}: no error measurements")
    return rows, skipped


def format_range(values):
    low, high = min(values), max(values)
    return f"{low:.6g}" if low == high else f"{low:.6g}..{high:.6g}"


def stability_details(rows):
    """Use recorded spacings or endpoint-node spacing on the configured domain."""
    if not all(math.isfinite(value) and value > 0
               for value in (DOMAIN_LENGTH_X, DOMAIN_LENGTH_Y)):
        raise ValueError("Domain lengths must be finite and positive")
    limits, sums = [], []
    for row in rows:
        if "dx" in row:
            dx, dy = row["dx"], row["dy"]
        else:
            dx = DOMAIN_LENGTH_X / (row["nx"] - 1)
            dy = DOMAIN_LENGTH_Y / (row["ny"] - 1)
        rate = row["thermal_diffusivity"] * (1 / dx**2 + 1 / dy**2)
        limit, stability_sum = 0.5 / rate, row["dt"] * rate
        if not all(math.isfinite(value) and value > 0
                   for value in (rate, limit, stability_sum)):
            raise ValueError("Invalid stability coefficients")
        limits.append(limit)
        sums.append(stability_sum)
    domain = ("recorded dx/dy" if "dx" in rows[0]
              else f"Lx={DOMAIN_LENGTH_X:g}, Ly={DOMAIN_LENGTH_Y:g}")
    return (f"{domain} | dt limit={format_range(limits)}"
            f" | rx+ry={format_range(sums)} (limit 0.5)")


def describe_plot(rows):
    """Select the scanned variable and describe the physical-time comparison."""
    first = rows[0]
    grid_fields = ("dx", "dy") if "dx" in first else ("nx", "ny")
    parameters = (*grid_fields, "thermal_diffusivity", "dt", "steps")
    varied = {name for name in parameters if len({row[name] for row in rows}) > 1}
    if varied <= {"steps"}:
        x_key, x_label = "steps", "Update steps"
        details = f"dt={first['dt']:g} | alpha={first['thermal_diffusivity']:g}"
        if "nx" in first:
            details += f" | grid={first['nx']} x {first['ny']}"
    elif varied <= set(grid_fields):
        x_key = next(name for name in grid_fields if name in varied)
        square = all(row[grid_fields[0]] == row[grid_fields[1]] for row in rows)
        if "nx" in first:
            x_label = ("Grid nodes per axis (nx = ny)" if square
                       else f"Grid nodes {x_key}")
        else:
            x_label = "Grid spacing h (dx = dy)" if square else f"Grid spacing {x_key}"
        time = first["dt"] * first["steps"]
        if not math.isfinite(time):
            raise ValueError("Non-finite final physical time")
        details = (f"dt={first['dt']:g} | steps={first['steps']} | final t={time:g}"
                   f" | alpha={first['thermal_diffusivity']:g}")
    elif "dt" in varied and varied <= {"dt", "steps"}:
        x_key, x_label = "dt", "Time step dt"
        times = [row["dt"] * row["steps"] for row in rows]
        if not all(math.isfinite(time) and time > 0 for time in times):
            raise ValueError("Final physical times must be finite and positive")
        fixed_time = all(math.isclose(time, times[0], rel_tol=1e-8, abs_tol=1e-14)
                         for time in times)
        if "steps" in varied and not fixed_time:
            raise ValueError("When dt and steps both change, dt * steps must stay fixed")
        if fixed_time:
            details = f"final t={times[0]:.6g} (fixed) | steps={format_range([r['steps'] for r in rows])}"
        else:
            details = (f"steps={first['steps']} (fixed)"
                       f" | final t={format_range(times)} (varies)")
            print("Note: dt varies with fixed steps, so final physical time varies; "
                  "this is not a fixed-final-time convergence comparison.", file=sys.stderr)
        if "nx" in first:
            details += f" | grid={first['nx']} x {first['ny']}"
    else:
        raise ValueError(f"Unsupported simultaneous parameter changes: {sorted(varied)}; "
                         "scan steps, refine the grid at fixed dt/steps, or scan dt at a fixed grid")
    if len({row[x_key] for row in rows}) != len(rows):
        raise ValueError(f"Expected one measurement per {x_key}")
    details += f"\n{stability_details(rows)}"
    return sorted(rows, key=lambda row: row[x_key]), x_key, x_label, details


def main(default_experiment=DEFAULT_EXPERIMENT):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--experiment", default=default_experiment, metavar="NAME",
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
        rows, skipped = read_results(result_path)
        rows, x_key, x_label, details = describe_plot(rows)
    except (OSError, ValueError, TypeError, OverflowError, ZeroDivisionError) as error:
        parser.error(str(error))

    x_values = [row[x_key] for row in rows]
    fig, ax = plt.subplots(figsize=(9, 5.5), layout="constrained")
    for metric, name, color, marker in (("mae", "MAE", "#2563eb", "o"),
                                       ("rmse", "RMSE", "#d97706", "x")):
        # Break lines at zero/non-finite values rather than inventing finite data.
        values = [row[metric] if math.isfinite(row[metric]) and row[metric] > 0
                  else math.nan for row in rows]
        ax.plot(x_values, values, color=color, marker=marker, markersize=4,
                linewidth=1.8, label=name)

    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlim(min(x_values) / 1.2, max(x_values) * 1.2)
    if not any(math.isfinite(row[key]) and row[key] > 0
               for row in rows for key in ("mae", "rmse")):
        ax.set_ylim(1, 10)
        ax.set_yticks([], minor=False)
        ax.set_yticks([], minor=True)
        ax.text(0.5, 0.5, "No positive finite errors to plot", transform=ax.transAxes,
                ha="center", va="center")
    for row in rows:
        count, mae, rmse = row[x_key], row["mae"], row["rmse"]
        nonfinite = [f"{name}={value:g}" for name, value in (("MAE", mae), ("RMSE", rmse))
                     if not math.isfinite(value)]
        zeros = [f"{name}=0" for name, value in (("MAE", mae), ("RMSE", rmse)) if value == 0]
        for labels, height, offset, alignment, marker in (
                (nonfinite, 0.97, -12, "top", "v"), (zeros, 0.03, 12, "bottom", "^")):
            if not labels:
                continue
            ax.plot([count], [height], marker=marker, color="#b91c1c",
                    transform=ax.get_xaxis_transform())
            message = f"{x_key}={count:g}: " + ", ".join(labels)
            ax.annotate(message, xy=(count, height), xycoords=ax.get_xaxis_transform(),
                        xytext=(-6, offset), textcoords="offset points", ha="right",
                        va=alignment, fontsize=9, color="#b91c1c",
                        bbox=dict(facecolor="white", alpha=0.85, edgecolor="none"))
            print(f"Log plot edge annotation: {message}", file=sys.stderr)

    ax.set_xlabel(x_label)
    ax.set_ylabel("Temperature error (log scale)")
    title = f"Heat diffusion error: {args.experiment}"
    if timestamp:
        title += f" | {timestamp}"
    title += f"\n{details}"
    ax.set_title(title, fontsize=11)
    ax.grid(True, alpha=0.25)
    ax.legend()

    suffix = f"_{timestamp}" if timestamp else ""
    figure_path = EXPERIMENT_DIR / "figures" / f"{args.experiment}{suffix}.png"
    figure_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(figure_path, dpi=200)
    print(f"Input: {result_path}")
    print(f"Horizontal axis: {x_key}")
    print(f"Parameter-only rows skipped: {skipped}; measurements: {len(rows)}")
    print(f"Figure written to: {figure_path}")
    if args.show:
        plt.show()
    plt.close(fig)


if __name__ == "__main__":
    main()
