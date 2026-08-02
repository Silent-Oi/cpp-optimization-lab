from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


SCRIPT_DIR = Path(__file__).resolve().parent
BENCHMARK_DIR = SCRIPT_DIR.parent

# 只从人工确认并冻结的 baseline CSV 绘图；benchmark 可执行文件的日常输出使用
# aos_benchmark.csv，避免一次试跑直接覆盖已提交的基线数据。
csv_path = BENCHMARK_DIR / "results" / "aos_benchmark.csv"
# PNG 先写为工作文件；目视确认后再与 CSV 一起另存为 *_baseline.png。
figure_path = BENCHMARK_DIR / "figures" / "aos_benchmark.png"

figure_path.parent.mkdir(parents=True, exist_ok=True)

data = pd.read_csv(csv_path)

plt.figure()
plt.plot(
    data["N"],
    data["average_ns"],
    marker="o",
    label="Average",
)

plt.plot(
    data["N"],
    data["median_ns"],
    marker="o",
    label="Median"
)

plt.xscale("log", base=2)
plt.xlabel("Oscillator count N")
plt.ylabel("Time per oscillator step (ns)")
plt.title("AoS batch performance")
plt.grid(True)
plt.legend(loc = "upper left")


plt.savefig(figure_path, dpi=200, bbox_inches="tight")
plt.show()

print(f"Figure written to: {figure_path}")
