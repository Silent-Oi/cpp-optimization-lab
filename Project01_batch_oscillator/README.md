# Project01 — Batch Oscillator

Project01 要做出两个相连的成果：

1. **批量欠阻尼振子计算程序**
2. **欠阻尼振子相空间星云**

它同时是整个仓库的 **C++ 性能优化入门项目** 和 **图形学入门项目**。

## 最终作品

### 批量计算

程序能够初始化并连续更新大量参数不同的欠阻尼振子。核心计算使用固定 `dt` 下的精确一步更新，使性能实验聚焦于数据布局、内存访问和 CPU 执行，而不是积分误差。

通过 AoS baseline、SoA、编译器自动向量化以及有意义时的简单多线程实验，理解同一算法为什么会因为数据组织和执行方式不同而产生性能差异。

### 相空间星云

将每个振子的状态映射到二维相空间：

```text
horizontal = position
vertical   = velocity / omega
```

大量振子形成旋转、展开并逐渐向原点收缩的星云。这个画面用于直接观察欠阻尼动力学，同时学习像素缓冲、坐标映射、窗口循环、动画和计算层/显示层分离。

## 物理模型

线性黏性阻尼振子：

```text
x' = v
v' = -2 * zeta * omega * v - omega^2 * x
```

核心范围：

- `omega > 0`
- `0 < zeta < 1`
- `zeta = 0` 用于无阻尼回归
- 所有振子使用统一 `dt`
- 不同振子可以拥有不同状态、`omega` 和 `zeta`

固定参数和固定 `dt` 下，每一步写成：

```text
x_next = a * x + b * v
v_next = c * x + d * v
```

四个系数在初始化时计算，热循环只更新状态。

## 固定技术路线

### 1. 单振子与正确性基线 — 已完成

- 无阻尼解析解与积分器学习
- 欠阻尼解析状态
- 固定 `dt` 的精确一步系数
- 单步、多步和零阻尼回归测试

### 2. AoS 批量 baseline — 已完成

- `std::vector<OscillatorAoS>` 连续存储
- 可复现初始化
- 单线程批量更新
- 与标量参考一致
- 大规模结果 checksum

### 3. AoS benchmark — 当前

- 合理控制不同规模的运行量
- 预热和重复测量
- 输出吞吐量与每次更新时间
- 观察性能随工作集规模的变化

当前入口见根目录 [`NOW.md`](../NOW.md)。

### 4. SoA 与编译器优化

- 只改变数据布局，不改变输入和算法
- 比较 AoS 与 SoA
- 检查编译器自动向量化
- 根据结果理解缓存、带宽和 SIMD

### 5. 条件多线程

只有单线程版本稳定、规模足够且并行确实有学习价值时，加入最简单的连续分块并行。线程池和复杂调度不属于本项目。

### 6. 相空间星云

- 建立独立 visualizer target
- 使用 CPU RGBA 像素缓冲
- 映射 `position` 与 `velocity / omega`
- 实现固定坐标范围、粒子绘制和短拖尾
- 加入最小窗口事件循环和暂停/重置
- 保持绘制代码与 `oscillator_core` 分离

具体实现路线见 [`docs/visualization_plan.md`](docs/visualization_plan.md)。

## 完成标准

Project01 完成时应当：

- 能正确更新百万级欠阻尼振子。
- 有 AoS 与 SoA 的实际性能比较。
- 能解释主要性能差异来自哪里，以及证据边界在哪里。
- 能运行相空间星云并看到旋转与阻尼收缩。
- 能解释核心计算、benchmark 和 visualizer 为什么分离。

不要求正式实验报告、CSV 数据库、完整 ODE 框架、临界/过阻尼支持、手写 SIMD 或通用图形框架。

## 代码结构

```text
Project01_batch_oscillator/
├─ include/       # 模型与批量接口
├─ src/           # 核心实现
├─ apps/          # 功能入口
├─ tests/         # 必要正确性检查
├─ benchmarks/    # 性能测量入口
├─ docs/
│  ├─ design.md
│  ├─ progress_checklist.md
│  ├─ visualization_plan.md
│  └─ learning_logs/   # 自愿学习记录
└─ CMakeLists.txt
```

主要 target：

```text
oscillator_core
oscillator_batch
oscillator_aos_benchmark
oscillator_tests
```

visualizer 实施时再加入独立 target，不提前建设图形框架。
