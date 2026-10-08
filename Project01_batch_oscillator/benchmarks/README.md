# Project01 性能实验

## 布局实验

比较相同初始输入、振子数量和更新步数下的 AoS 与 SoA 无终止更新。
`oscillator_benchmark` 在一次运行开始时生成统一时间标识，将当前测量写入：

- `layout_experiment/results/aos_benchmark.csv<时间标识>`
- `layout_experiment/results/soa_benchmark.csv<时间标识>`

时间标识格式为 `YYYYMMDD_HHMMSS`，例如 `20261008_160938`。比较历史结果时，需要核对步数、构建配置与硬件设置。

## 绘图

需要 Python 和 Matplotlib；安装依赖：

```powershell
python -m pip install matplotlib
```

在仓库根目录运行：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_layout_experiment.py
```

脚本使用相对于自身位置的路径，也可以从其他工作目录运行。
默认选择时间标识最新的完整 AoS/SoA 配对，图片保存到 `layout_experiment/figures/aosvsoa_benchmark_<时间标识>.png`。如果更新的结果缺少一份 CSV，脚本给出提示并使用上一组完整结果。

也可以指定一次实验：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_layout_experiment.py --timestamp 20261008_160938
```

添加 `--show` 可同时打开窗口。脚本兼容 `aos_benchmark_<时间标识>.csv` 和 `soa_benchmark_<时间标识>.csv` 的命名方式；仅在没有带时间标识的文件时，读取原来的 `aos_benchmark.csv`、`soa_benchmark.csv` 并生成无时间标识的图片。

同一种颜色表示同一种布局，实线表示平均值，虚线表示中位数。
横轴为振子数量（以 2 为底的对数刻度），纵轴为每振子每步耗时，越低越快。
脚本检查两组数据的 N、步数是否匹配，但不验证测量时的构建配置或硬件条件，也不会重新运行实验。

CSV 和图片是测量快照，不自动视为新的 baseline。不同时间标识的结果分别保存；同秒运行 benchmark 仍可能覆盖同名 CSV，重复绘制同一次实验会覆盖对应图片。

### AoS 大小实验绘图

在仓库根目录运行：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_aos_size_experiment.py
```

脚本从 `aos_size_experiment/results/` 选择时间标识最新的完整 `aos64_benchmark`、`aos68_benchmark` 配对，支持 `.csv<时间标识>` 和 `_<时间标识>.csv` 两种命名方式。图片保存到 `aos_size_experiment/figures/aos_size_benchmark_<时间标识>.png`。
同样支持 `--timestamp YYYYMMDD_HHMMSS` 和 `--show`；更新的结果不完整时提示并使用上一组完整结果。没有带时间标识的文件时，读取 `aos64_benchmark.csv`、`aos68_benchmark.csv`。

绘图风格与布局实验一致，并检查两组数据的 N、步数是否匹配，以及每份 CSV 的结构体大小是否固定。图例和标题使用 CSV 的 `sizeof` 列，不根据文件名推断实际字节数；较小的结构体使用蓝色，较大的使用橙色。

### 终止机制实验绘图

在仓库根目录运行：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_termination_experiment.py
```

脚本从 `termination_experiment/results/` 选择时间标识最新的完整 `soa_benchmark`、`soa_no_termination_benchmark` 配对，支持 `.csv<时间标识>` 和 `_<时间标识>.csv` 两种命名方式。图片保存到 `termination_experiment/figures/termination_benchmark_<时间标识>.png`。
同样支持 `--timestamp YYYYMMDD_HHMMSS` 和 `--show`；更新的结果不完整时提示并使用上一组完整结果。没有带时间标识的文件时，读取 `soa_benchmark.csv`、`soa_no_termination_benchmark.csv`。

横轴为请求的更新步数（以 2 为底的对数刻度），蓝色表示有终止，橙色表示无终止，实线表示平均值，虚线表示中位数。脚本检查两组数据的 Steps、N 是否匹配，并要求振子数量 N 固定。
纵轴直接使用 CSV 的耗时，归一化分母为初始振子数 N × 请求步数 Steps。有终止版本会跳过已终止振子的更新，因此该指标用于比较相同请求步数下的运行成本，不代表每次实际执行更新的耗时；两种版本的最终状态也可能因终止近似而不同。

### 向量化实验绘图

在仓库根目录运行：

```powershell
python Project01_batch_oscillator/benchmarks/scripts/plot_vectorization_experiment.py
```

脚本从 `vectorization_experiment/results/` 选择时间标识最新的完整 `soa_benchmark`、`soa_scalar_benchmark` 配对，支持 `.csv<时间标识>` 和 `_<时间标识>.csv` 两种命名方式。图片保存到 `vectorization_experiment/figures/vectorization_benchmark_<时间标识>.png`。
同样支持 `--timestamp YYYYMMDD_HHMMSS` 和 `--show`；更新的结果不完整时提示并使用上一组完整结果。没有带时间标识的文件时，读取 `soa_benchmark.csv`、`soa_scalar_benchmark.csv`。

横轴为振子数量（以 2 为底的对数刻度），纵轴为每振子每步耗时。蓝色表示 SoA 默认路径，橙色表示 SoA scalar 路径，实线表示平均值，虚线表示中位数。脚本检查两组数据的 N、步数是否匹配，并要求更新步数固定。
曲线名称对应 benchmark 调用入口；是否实际生成向量指令或保持标量执行，需要结合编译器诊断或汇编确认，CSV 和绘图脚本不验证这一点。

## 后续实验与结果保存

- AoS 大小实验比较普通结构体和带额外字段的结构体，保持更新公式和输入一致；本轮不加入 packed 布局或专门的非对齐访问实验。
- 被后续测量取代或确认无效的实验输出，可连同图片放入 `benchmarks/archive/<实验名>/<时间标识>/`，附简短说明，区分归档原因、已知条件和未确定的问题。有参考价值的历史 baseline 继续保留；仅凭性能不符合预期，不判定数据无效。
- 个人学习记录保持作者原有组织与表达，不因结果归档自动移动或改写。
