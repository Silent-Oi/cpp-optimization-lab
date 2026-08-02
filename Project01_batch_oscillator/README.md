# Project01 — Batch Oscillator

## 项目定位

Project01 是本仓库第一个完整的“数学模型—正确实现—结果验证—性能测量—优化实验”项目。

它使用无阻尼简谐振子完成数值积分入门，再使用约百万个相互独立的线性阻尼振子建立 CPU 批量计算实验。项目重点不是收集尽可能多的积分器，而是学习如何把正确性、连续内存、数据布局、编译器优化和可复现测量连接起来。

本项目为后续二维热传导提供以下基础：

- 把物理状态和只读参数映射到 C++ 数据结构。
- 使用解析结果或独立参考验证数值程序。
- 建立 Release benchmark 和结果校验值。
- 使用数据而不是直觉判断优化是否有效。

## 学习目标

### 主学习目标

理解批量数值更新中的数据组织和 CPU 执行效率，并完成至少一个有证据支持的性能实验。

### 辅助学习目标

- 理解无阻尼数值误差与阻尼物理耗散的区别。
- 理解 `struct`、连续容器、引用传递和状态/参数分离在数值程序中的作用。
- 初步理解缓存、自动向量化和简单数据并行。

## 物理模型

最终模型是带线性黏性阻尼的一维振子：

```text
m x'' + c x' + k x = 0
```

使用固有角频率 \(\omega=\sqrt{k/m}\) 和阻尼比
\(\zeta=c/(2\sqrt{km})\) 后：

```text
dx/dt = v
dv/dt = -2*zeta*omega*v - omega^2*x
```

第一版只处理欠阻尼 `0 < zeta < 1`，并允许 `zeta = 0` 作为无阻尼回归情况。临界阻尼和过阻尼不属于核心范围。

对于固定时间步长和不随时间变化的参数，每个振子可以写成精确的一步线性更新：

```text
[x_(n+1)]   [a b] [x_n]
[v_(n+1)] = [c d] [v_n]
```

主批量版本在初始化阶段根据 `omega`、`zeta` 和 `dt` 预计算每个振子的更新系数，在热循环中只执行状态读取、线性组合和状态写回。这既避免把数值积分误差混入主要性能实验，也形成适合研究 AoS/SoA、缓存和向量化的稳定工作负载。

## 范围边界

### 核心范围

- 收尾现有无阻尼解析解与积分器的最低正确性验证。
- 建立单个欠阻尼振子的解析参考和精确一步更新。
- 建立任意数量振子的标量 AoS baseline。
- 建立可复现 benchmark，并先判断主要瓶颈。
- 公平比较 AoS 与 SoA，检查编译器自动向量化。
- 在项目最终总结前完成 V1 实时振子点阵，使批量状态和阻尼衰减能够被直接观察。
- 形成至少一份可复现实验报告和最终学习总结。

### 条件触发的实验

- **实时计算与预计算系数**：只有 baseline 测量稳定后才比较计算量与额外参数读取。
- **简单多线程分块**：只有单线程 benchmark 稳定、工作规模足够且核心循环仍值得并行时进行。
- **手动 SIMD**：只有自动向量化证据不足、核心循环仍为计算瓶颈且实验具有明确学习价值时进行。
- **RK4 或完整收敛阶实验**：只有需要继续研究通用积分方法时进行，不阻塞批量性能主线。

### 明确不做

- 通用 ODE 求解器框架。
- 临界阻尼和过阻尼的完整实现。
- 线程池、无锁结构、自定义分配器或复杂 SIMD 封装。
- 为了复用而提前把振子领域对象放入 `Project00_common`。

## 正确性策略

- 无阻尼解析解验证现有积分器的初始状态、周期和单步公式。
- 欠阻尼解析解验证直接状态计算和精确一步更新。
- `zeta = 0` 时结果应退化为无阻尼模型。
- 连续阻尼模型的机械能满足非增趋势；必须区分物理耗散和实现误差。
- 批量结果必须与逐个标量更新一致。
- AoS、SoA 和可选多线程版本必须在约定容差内一致。
- 性能计时始终保留结果校验值，防止核心计算被编译器消除。

## 可观察性与可视化

V1 的目标不是用画面证明 AoS、SoA 或其他优化方案更快，而是把 Project01 的研究对象——大量独立欠阻尼振子——转化为具有明确物理含义的动态画面。

核心展示成果是 **阻尼振子相空间星云**。每个振子作为相空间中的一个发光粒子，`position` 决定水平位置，`velocity / omega` 决定垂直位置。不同 `omega` 产生不同的相位推进速度，不同 `zeta` 产生不同的收缩速度；大量粒子共同形成旋转、展开并逐渐向原点坍缩的星云结构。深色背景和短暂拖尾用于直接表现振荡轨迹与阻尼衰减。

在核心成果完成后，V1 计划增加 **参数梯度发光幕布**：把振子按照 `omega` 或 `zeta` 顺序排列，以当前 `position` 形成连续发光曲线，观察参数连续变化如何改变整组独立振子的运动形态。相邻点之间的连线只用于视觉表达，不表示振子之间存在物理耦合。

visualizer 只作为 `oscillator_core` 的独立使用者，不复制振子算法。tests 继续负责正确性验证，benchmark 继续负责性能测量；绘制、拖尾、事件轮询和帧率均不进入正式性能结论。

实际学习顺序为先完成 M3，再完成与其假设直接相连的 M4，随后连续完成 V1，并在 M6 前收口。V1 的详细目标、视觉语义、边界和完成门槛见 [`docs/visualization_plan.md`](docs/visualization_plan.md)。

## 核心里程碑

1. **无阻尼学习基线收尾**：现有实现具备最低限度的自动验证，并完成一次聚焦的长期行为记录。
2. **单个阻尼振子参考**：解析状态、精确一步系数和零阻尼回归可信。
3. **批量 AoS baseline**：任意数量振子可以连续存储和正确更新。
4. **Benchmark 与瓶颈证据**：测量流程可复现，并能说明核心循环更可能受计算、缓存还是带宽限制。
5. **数据布局与编译器实验**：AoS/SoA 和自动向量化结论有公平对比支持。
6. **条件并行实验**：满足进入条件时研究简单分块和线程扩展性；不满足时记录不做的原因。
7. **项目总结**：保留正确性证据、实验报告、主要结论和可迁移经验。

V1 不插入或重新编号上述性能里程碑。架构上它与性能实验相互独立；实际学习顺序为先完成 M3，再完成 M4，随后完成 V1，并在 M6 项目总结前达到自身核心完成标准。条件并行实验若被触发，应另行安排而不与 V1 同时推进。

详细状态见 [`docs/progress_checklist.md`](docs/progress_checklist.md)。

## 主要性能问题

Project01 优先回答三个问题：

1. 对同一批阻尼振子更新，AoS 与 SoA 的性能是否随数据规模和缓存层级发生变化？
2. 预计算更新系数减少了多少计算，又增加了多少内存读取；当前机器上哪一侧更重要？
3. 编译器是否能够自动向量化核心循环，数据布局是否改变向量化结果？

多线程只在前述问题已有可信结果后进入。

## 构建

在仓库根目录执行：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B out\build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build out\build\x64-Release'
```

当前构建目标及 Release 可执行文件：

```text
harmonic_oscillator    -> out\build\x64-Release\Project01_batch_oscillator\harmonic_oscillator.exe
underdamped_oscillator -> out\build\x64-Release\Project01_batch_oscillator\underdamped_oscillator.exe
oscillator_batch       -> out\build\x64-Release\Project01_batch_oscillator\oscillator_batch.exe
oscillator_aos_benchmark
                       -> out\build\x64-Release\Project01_batch_oscillator\oscillator_aos_benchmark.exe
oscillator_tests       -> out\build\x64-Release\Project01_batch_oscillator\oscillator_tests.exe
```

`oscillator_batch` 是 M2 的批量功能与压力运行入口，`oscillator_tests` 是独立测试入口。
`oscillator_aos_benchmark` 是 M3 的 AoS benchmark 驱动；正式性能结论以可复现实验报告为准。

## 文档与目录

```text
Project01_batch_oscillator/
├─ include/                   # 状态、模型和时间更新接口
├─ src/                       # 可复用的模型和更新实现
├─ apps/                      # 单振子与 AoS 批量功能/压力运行入口
├─ benchmarks/                # 独立的性能测量驱动
├─ tests/                     # 正确性与回归测试
├─ docs/
│  ├─ progress_checklist.md   # 当前状态与完成证据
│  ├─ design.md               # 已作出的设计决定
│  ├─ visualization_plan.md   # V1 唯一详细实施路线
│  ├─ diagrams/               # 源码结构、数据流与 benchmark 流程图
│  ├─ experiment_plan.md      # Project01 特有实验顺序
│  ├─ learning_logs/          # 按里程碑保存的学习者原始记录与附件
│  │  ├─ M0/
│  │  │  └─ learning_log.md
│  │  ├─ M1/
│  │  │  ├─ learning_log.md
│  │  │  └─ assets/
│  │  └─ M2/
│  │     └─ learning_log.md
│  └─ experiments/            # 可提交的实验报告
├─ results/                   # 本地原始实验输出，默认忽略
└─ CMakeLists.txt
```
