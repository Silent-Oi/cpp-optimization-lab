# Project02 数值实验

## 误差实验：通用绘图

需要 Python 和 Matplotlib。安装依赖后，在仓库根目录运行：

```powershell
python -m pip install matplotlib
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py
```

在 [plot_error_experiment.py](scripts/plot_error_experiment.py) 顶部修改默认实验名即可选择常用实验，只填写文件名前缀，不带时间标识或 `.csv`：

```python
DEFAULT_EXPERIMENT = "grid_refinement_experiment"
```

也可以临时用命令行覆盖，无需修改脚本：

```powershell
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment unstable_dt_experiment
python Project02_heat_equation/benchmarks/scripts/plot_error_experiment.py --experiment grid_refinement_experiment
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

横轴以 2 为底取对数，纵轴为温度误差的对数。每个横坐标只允许一组测量。时间步与网格同时变化、时间步细化等其他扫描目前会明确报告不支持，避免错误地画成步数或网格扫描。

蓝色表示 MAE，橙色表示 RMSE。零值和非有限值无法作为普通对数坐标点：零值在图的下边缘标注，`inf`/`nan` 在上边缘标注，其标记高度不表示误差大小，曲线在这些位置断开。

旧格式步数扫描的标题根据 CSV 参数计算显式五点差分的稳定条件 `rx + ry <= 0.5` 及时间步上限：

```text
rx + ry = thermal_diffusivity * dt * (1/dx^2 + 1/dy^2)
dt_limit = 0.5 / (thermal_diffusivity * (1/dx^2 + 1/dy^2))
```

这些值使用 CSV 中已舍入的参数。只有 `nx、ny` 的 CSV 没有记录物理区域长度，脚本不从节点数量推断 `dx、dy` 或稳定上限；网格扫描图也不标注单一稳定上限。

脚本只展示已有误差结果，不重新运行求解器，不验证不同网格的物理区域是否相同；RMSE 为 `inf` 也可能来自误差平方求和溢出，不能仅凭该指标判定温度数组已包含非有限值。
