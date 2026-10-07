# Project01 性能实验

## 布局实验

比较相同初始输入、振子数量和更新步数下的 AoS 与 SoA 无终止更新。
`oscillator_benchmark` 将当前测量写入：

- `layout_experiment/results/aos_benchmark.csv`
- `layout_experiment/results/soa_benchmark.csv`

原来的 `results/*_baseline.csv` 保留为历史结果。比较历史结果时，需要核对步数、构建配置与硬件设置。

## 绘图

需要 Python 和 Matplotlib；安装依赖：

```powershell
python -m pip install matplotlib
```

在仓库根目录运行：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_aos_benchmark.py
```

脚本使用相对于自身位置的路径，也可以从其他工作目录运行。
图片保存到 `layout_experiment/figures/aosvsoa_benchmark.png`；添加 `--show` 可同时打开窗口。

同一种颜色表示同一种布局，实线表示平均值，虚线表示中位数。
横轴为振子数量（以 2 为底的对数刻度），纵轴为每振子每步耗时，越低越快。
脚本检查两组数据的 N、步数是否匹配，但不验证测量时的构建配置或硬件条件，也不会重新运行实验。

CSV 和图片是当前测量快照，不自动视为新的 baseline。运行 benchmark 会覆盖当前 CSV，运行绘图脚本会覆盖对应图片。
AoS 大小实验及终止机制实验尚未接入此绘图脚本。
