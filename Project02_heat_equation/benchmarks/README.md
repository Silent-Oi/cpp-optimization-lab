# Project02 数值实验

## 时间步超过稳定限制：误差绘图

需要 Python 和 Matplotlib。安装依赖后，在仓库根目录运行：

```powershell
python -m pip install matplotlib
python Project02_heat_equation/benchmarks/scripts/plot_unstable_dt_experiment.py
```

脚本默认读取 `error_experiment/results/` 下时间标识最新的 `unstable_dt_experiment<时间标识>`，也支持 `_<时间标识>.csv`、`<时间标识>.csv` 和 `.csv<时间标识>` 后缀。路径相对于脚本位置解析，可以从其他工作目录运行。

可用 `--timestamp YYYYMMDD_HHMMSS` 选择历史实验，`--show` 同时打开图形窗口。图片保存到 `error_experiment/figures/unstable_dt_experiment_<时间标识>.png`；重复绘制同一次实验会覆盖对应图片。

输入列为 `dx,dy,thermal_diffusivity,dt,steps,mae,rmse`。参数可只出现在配置行，也可在测量行重复，但同一次实验内必须保持一致。仅含配置、没有误差值的行不作为测量点；一行只缺少一种误差指标时会报告错误。

横轴为更新步数，以 2 为底的对数刻度；纵轴为温度误差，取对数。蓝色表示 MAE，橙色表示 RMSE。零值和非有限值无法作为普通对数坐标点：零值在图的下边缘标注，`inf`/`nan` 在上边缘标注，其标记高度不表示误差大小，曲线在这些位置断开。

标题根据 CSV 参数计算显式五点差分的稳定条件 `rx + ry <= 0.5` 及时间步上限：

```text
rx + ry = thermal_diffusivity * dt * (1/dx^2 + 1/dy^2)
dt_limit = 0.5 / (thermal_diffusivity * (1/dx^2 + 1/dy^2))
```

这些值使用 CSV 中已舍入的参数。脚本只展示已有误差结果，不重新运行求解器；RMSE 为 `inf` 也可能来自误差平方求和溢出，不能仅凭该指标判定温度数组已包含非有限值。
