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
- 在 V1 中使用 OpenGL、Vulkan、DirectX、shader、GPU 绘制、GUI 框架、字体系统、多窗口或复杂参数编辑界面。
- 通用 renderer、scene、window wrapper、图形抽象层，或提前把窗口与软件绘制代码放入 `Project00_common`。

## 正确性策略

- 无阻尼解析解验证现有积分器的初始状态、周期和单步公式。
- 欠阻尼解析解验证直接状态计算和精确一步更新。
- `zeta = 0` 时结果应退化为无阻尼模型。
- 连续阻尼模型的机械能满足非增趋势；必须区分物理耗散和实现误差。
- 批量结果必须与逐个标量更新一致。
- AoS、SoA 和可选多线程版本必须在约定容差内一致。
- 性能计时始终保留结果校验值，防止核心计算被编译器消除。

## 可观察性与可视化

### 定位与时序

Project01 增加独立并行支线 **V1 — 实时可视化与软件绘制入门**。建议在 M3 完成后开始，最迟在 M6 项目总结前完成。

V1 是 Project01 最终必须留下的核心成果之一，但不是 M3 benchmark、M4 数据布局实验或后续性能项目的正确性与技术前置条件。性能主线仍按 M3 → M4 → 条件并行实验 → 项目总结推进；V1 不承担 AoS/SoA 性能比较，也不能用画面流畅度形成核心计算性能结论。

### 最终最小成果：实时振子点阵

最终画面不是只显示一个振子，而是使用现有批量计算得到实时振子点阵：

- 屏幕划分为规则网格，每个显示单元代表一个被抽样的振子。
- 单元中心表示平衡位置，光点相对中心的水平位移表示 `position`。
- 颜色或亮度表示 `velocity` 的方向和大小。
- 随模拟推进，可以直接观察不同振子的阻尼振荡和逐渐衰减。
- 核心计算可以使用百万规模 batch，显示端只绘制与窗口大小匹配的数百至一千个代表性振子。

计算数量与显示数量必须分离：

```text
计算：完整 batch，可达到百万规模
显示：固定数量的代表性振子，依据窗口大小确定
```

显示索引必须均匀、固定、可复现并覆盖整个 batch，不能默认只取前若干个元素。单个振子动画只是接入核心计算和校准坐标映射的中间步骤，不是 V1 最终成果。

### 第一版技术路线

- 使用 RGFW 创建窗口、处理输入事件并显示像素缓冲。
- 使用 CPU 软件绘制和 RGBA 像素缓冲，通过 `RGFW_surface` 与窗口 blit 显示结果。
- RGFW 只属于未来的 `oscillator_visualizer` target，不进入 `oscillator_core`。
- 实施 V1 时固定 RGFW 版本或 commit，并保留对应许可证；具体依赖导入方式到 V1 开始时确认。
- RGFW implementation 只在一个翻译单元中实例化，visualizer 自己维护最小的软件绘制代码。

`oscillator_visualizer` 目前只是规划中的 target；本次没有添加 RGFW、源码、目录或 CMake 配置。

### 职责边界

规划中的依赖关系为：

```text
oscillator_core
├── apps
├── tests
├── benchmark
└── oscillator_visualizer

oscillator_visualizer
├── 调用 oscillator_core
├── RGFW 窗口与事件
└── visualizer 自己的软件绘制代码
```

- visualizer 直接复用现有批量初始化和更新接口，不复制振子公式或维护第二套算法。
- 核心数据类型不包含 RGFW、窗口、颜色、像素或帧记录类型；抽样索引、坐标、颜色和显示状态由 visualizer 管理。
- 只有实际实现证明现有只读访问方式不足时，才讨论最小核心观察接口；规划阶段不设计 snapshot 框架。
- 绘制、事件轮询、帧率限制、截图和交互全部位于正式 benchmark 计时之外。
- AoS/SoA、向量化和线程扩展性继续由独立 benchmark 以固定条件测量。

### 模拟时间与最小交互

V1 必须区分固定物理时间步 `dt`、模拟推进次数、现实经过时间和画面刷新帧率。第一版可以每帧推进固定步数，但完成 V1 前应理解并记录固定时间步 accumulator 的作用，不能让显示器帧率静默改变模拟时间语义，也不能为了维持帧率修改核心物理公式。

核心交互限定为：

- `Space`：暂停与继续。
- `Right Arrow`：暂停时单步推进。
- `R`：恢复相同 seed 和参数下的初始状态。
- `Escape`：退出。

### V1 高层完成标准

1. 建立最小 RGFW 窗口并显示纯色 RGBA 像素缓冲。
2. 能够清屏、安全写入像素并绘制小型光点或矩形。
3. 使用现有核心接口完成单个振子的坐标映射和阻尼衰减观察。
4. 使用完整 batch 计算和确定性抽样完成实时振子点阵，并记录实际可运行规模或硬件限制。
5. 完成暂停、单步、重置和退出，保存至少一张代表截图以及一段 GIF 或短视频。
6. 在实现完成后更新 README 的实际构建、运行和观看说明。

正式运行展示素材计划保存在 `docs/showcase/`；该目录目前尚未创建。这里的程序截图、GIF 和短视频不同于 `docs/diagrams/` 中的设计图，也不同于 `docs/learning_logs/<里程碑>/assets/` 中的学习过程附件。

### 主画面完成后的条件扩展

只有 V1 核心成果已经完成，并且扩展仍有明确学习价值时，才考虑：

- 选定振子的相空间轨迹。
- 无阻尼和阻尼行为的并列对比。
- 位移、能量或振幅衰减曲线。
- 按 `omega` 或 `zeta` 映射颜色。
- 更复杂的交互参数编辑。
- GPU 绘制。

## 核心里程碑

1. **无阻尼学习基线收尾**：现有实现具备最低限度的自动验证，并完成一次聚焦的长期行为记录。
2. **单个阻尼振子参考**：解析状态、精确一步系数和零阻尼回归可信。
3. **批量 AoS baseline**：任意数量振子可以连续存储和正确更新。
4. **Benchmark 与瓶颈证据**：测量流程可复现，并能说明核心循环更可能受计算、缓存还是带宽限制。
5. **数据布局与编译器实验**：AoS/SoA 和自动向量化结论有公平对比支持。
6. **条件并行实验**：满足进入条件时研究简单分块和线程扩展性；不满足时记录不做的原因。
7. **项目总结**：保留正确性证据、实验报告、主要结论和可迁移经验。

V1 不插入或重新编号上述性能里程碑。它可以在 M3 完成后与 M4 或后续工作并行，但必须在项目总结完成前达到自身核心完成标准。

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
规划中的 `oscillator_visualizer` 尚不是当前 CMake target；应在 V1 实施开始后再添加和记录实际构建、运行命令。

## 文档与目录

```text
Project01_batch_oscillator/
├─ include/                   # 状态、模型和时间更新接口
├─ src/                       # 可复用的模型和更新实现
├─ apps/                      # 单振子与 AoS 批量功能/压力运行入口
├─ benchmarks/                # 独立的性能测量驱动
├─ tests/                     # 正确性与回归测试
├─ visualization/             # 独立可视化模块
├─ third_party/               # 第三方依赖
│  └─ RGFW/ 
├─ docs/
│  ├─ progress_checklist.md   # 当前状态与完成证据
│  ├─ design.md               # 已作出的设计决定
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

V1 计划新增的 `docs/showcase/` 当前也不属于现有目录；只有开始保存正式运行展示素材时才创建。
