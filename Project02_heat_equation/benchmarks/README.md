# Project02 数值实验

## 多线程实验绘图

需要 Python 和 Matplotlib。在仓库根目录运行：

```powershell
python Project02_heat_equation/benchmarks/scripts/plot_parallel_experiment.py
```

脚本默认读取 `parallel_experiment/results/` 下最新的 `parallel_experiment<时间标识>`，生成 `parallel_experiment/figures/parallel_experiment_<时间标识>.png`。支持与误差绘图相同的文件名后缀、`--timestamp`、`--show`，也可以修改顶部 `DEFAULT_EXPERIMENT` 或使用 `--experiment NAME` 选择其他同格式实验。

输入列为 `threads,nx,ny,thermal_diffusivity,dt,steps,ns_per_calu`。要求每个线程数一条完整记录，网格、扩散系数、时间步和步数在扫描期间固定，参数和耗时均为正数且有限，并包含 1 线程结果。

左图显示 CSV 中记录的单次耗时指标，归一化分母为 `nx × ny × steps`，包含固定边界节点，不能称为每个内部节点的更新时间。右图显示 `time(1 thread)/time(p threads)`，基线是同一并行求解接口的 1 线程调用；虚线为理想线性加速参考。当前 CSV 每种线程数只有一条耗时，图中不将它标为平均值或中位数。

当前输出中的前两列可能合并为 `1 512`。脚本仅对表头为 `threads,nx,...`、缺少一个分隔符且首字段可明确拆成两个整数的行进行兼容读取，并打印提示，不修改源文件。标准逗号格式也可直接读取。早期仅在第一行输出参数、后续只有裸耗时的文件缺少线程映射，脚本会报错，不推断缺失参数。

绘图只消费已有结果，不重新计时或验证数值正确性，也不验证运行时的构建配置和硬件条件。

## 误差实验：通用绘图

需要 Python 和 Matplotlib。安装依赖后，在仓库根目录运行：

```powershell
python -m pip install matplotlib
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py
```

在 [plot_error_experiment.py](scripts/plot_error_experiment.py) 顶部修改默认实验名即可选择常用实验，只填写文件名前缀，不带时间标识或 `.csv`：

```python
DEFAULT_EXPERIMENT = "time_step_refinement_experiment"
```

也可以临时用命令行覆盖，无需修改脚本：

```powershell
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment unstable_dt_experiment
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment grid_refinement_experiment
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment time_step_refinement_experiment
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment base_line --timestamp 20261021_192300
```

脚本只在指定实验名下选择最新结果，例如 `base_line20261021_192300`，也支持 `_<时间标识>.csv`、`<时间标识>.csv` 和 `.csv<时间标识>` 后缀。路径相对于脚本位置解析，可以从其他工作目录运行。没有带时间标识的结果时，尝试同名的无后缀文件或 `.csv` 文件；找不到指定实验会报告错误，不会自动改选其他实验。

`--timestamp YYYYMMDD_HHMMSS` 选择该实验的历史结果，`--show` 同时打开图形窗口。标题包含实验名，图片保存到 `error_experiment/figures/<实验名>_<时间标识>.png`；重复绘制同一次实验会覆盖对应图片。

原来的 `plot_unstable_dt_experiment.py` 可继续绘制旧格式的 `unstable_dt_experiment`；新增实验使用通用入口。

支持两种输入列：

- 旧格式：`dx,dy,thermal_diffusivity,dt,steps,mae,rmse`
- 节点数格式：`nx,ny,thermal_diffusivity,dt,steps,mae,rmse`

空白参数沿用上一行，因此支持首行集中配置、后续只输出变化参数的布局。仅含配置、没有误差值的行不作为测量点；一行只缺少一种误差指标时会报告错误。

脚本根据测量行中实际变化的参数选择横轴：

- 固定网格、扩散系数和时间步，仅改变步数：横轴为 `steps`。
- 固定扩散系数、时间步和步数，仅改变网格：横轴优先取变化的 `nx`（旧格式取 `dx`）；仅 y 方向变化时取 `ny` 或 `dy`。方形网格标注 `nx = ny`，等间距网格标注 `dx = dy`。标题注明固定步数与 `final t = dt * steps`。
- 固定网格和扩散系数、改变时间步：横轴为 `dt`。步数固定时，标题显示变化的终止时刻范围，并提示这不是同一终止时刻的时间收敛比较。也支持同步调整步数、保持 `dt * steps` 不变的扫描，此时标题标注固定终止时刻；终止时刻一致性的相对容差为 `1e-8`，绝对容差为 `1e-14`。

横轴以 2 为底取对数，纵轴为温度误差的对数。每个横坐标只允许一组测量。网格与时间步同时变化等其他扫描会明确报告不支持，避免误用横轴。

蓝色表示 MAE，橙色表示 RMSE。零值和非有限值无法作为普通对数坐标点：零值在图的下边缘标注，`inf`/`nan` 在上边缘标注，其标记高度不表示误差大小，曲线在这些位置断开。

当前矩形区域的两个方向长度均为 1。节点数格式的 CSV 使用脚本顶部的 `DOMAIN_LENGTH_X = 1.0`、`DOMAIN_LENGTH_Y = 1.0`，按包含两端点的网格计算 `dx = Lx/(nx-1)`、`dy = Ly/(ny-1)`；旧格式使用 CSV 已记录的 `dx、dy`。物理区域尺寸改变时需同步修改这两个常量。

标题据此计算显式五点差分的稳定条件 `rx + ry <= 0.5` 及时间步上限：

```text
rx + ry = thermal_diffusivity * dt * (1/dx^2 + 1/dy^2)
dt_limit = 0.5 / (thermal_diffusivity * (1/dx^2 + 1/dy^2))
```

这些值使用 CSV 中已舍入的参数与配置的物理区域长度。多个测量点的稳定上限或 `rx+ry` 不同时，标题展示其最小值到最大值。

脚本只展示已有误差结果，不重新运行求解器，不验证不同网格的物理区域是否相同；RMSE 为 `inf` 也可能来自误差平方求和溢出，不能仅凭该指标判定温度数组已包含非有限值。
