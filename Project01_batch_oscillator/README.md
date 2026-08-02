# Project01 — Batch Oscillator

> Million-scale damped oscillator simulation, CPU performance analysis, and phase-space visualization.

项目从批量更新大量欠阻尼振子出发，研究数据布局、缓存、向量化和多线程如何影响 CPU 性能，并最终把振子状态绘制成实时相空间星云。

项目目标：

> **高性能大量欠阻尼振子运作状态演化**

## 目标实现

Project01 由两个相互连接的部分组成。

### 批量振子计算

程序能够初始化并连续更新大量参数不同的欠阻尼振子。

### 相空间星云

每个振子被映射到二维相空间：

```text
horizontal = position
vertical   = velocity / omega
```

大量振子共同形成旋转、展开并逐渐向原点收缩的星云。


## 项目设计
[`docs/design.md`](docs/design.md)

## 性能实验

### 已完成实验

Benchmark 通过计算每个振子单步更新需要的时间来判断性能


1. AoS 与 SoA 数据布局对性能的影响
2. 输入数据规模对性能的影响

### 计划实验

1. 数据布局设计对性能的影响
2. Cache blocking 对多步批量更新的缓存局部性和性能影响
3. 多线程对性能的影响

## 当前进展

已经完成：

- 谐振子数值方法实验
- 欠阻尼解析参考
- 固定时间步的精确一步更新
- 单步、多步和无阻尼回归测试
- 可复现的批量初始化
- AoS 批量实现
- SoA 批量实现
- AoS、SoA 与解析参考的一致性测试
- AoS 与 SoA benchmark
- CSV 输出与绘图流程
- 部分性能实验

后续内容：

- 计划性能实验
- 编译器自动向量化分析
- 多线程批量更新与扩展性测试
- 相空间坐标映射
- CPU 像素缓冲
- 实时相空间星云

## 技术地图

| 技术 | 需要实现的东西 | 目的 |
|---|---|---|
| 解析参考 | 任意时刻的单振子状态 | 建立正确性基准 |
| 精确一步更新 | 预计算 2×2 更新系数 | 从热循环中移除积分误差和昂贵函数 |
| 连续存储 | `std::vector` 批量容器 | 高效处理大量数据 |
| AoS | 每个振子一个结构体 | 建立直观 baseline |
| Benchmark | 数据规模与单次更新时间曲线 | 观察缓存和内存阶段 |
| SoA | 每个字段一个连续数组 | 改善数据流局部性和 SIMD 条件 |
| 自动向量化 | 编译器报告与核心循环分析 | 理解单核数据并行 |
| 多线程 | 连续区间分块更新 | 使用多个 CPU 核心 |
| 相空间映射 | `(x, v / omega)` 到屏幕坐标 | 把状态转成可见几何 |
| 像素缓冲 | CPU 生成 RGBA 图像 | 产生画面 |
| 窗口循环 | 更新、绘制、显示 | 将仿真变成实时动画 |

## 项目结构

```text
Project01_batch_oscillator/
├─ include/       # 模型与公共批量接口
├─ src/           # 核心振子实现
├─ apps/          # 功能示例入口
├─ tests/         # 正确性与回归测试
├─ benchmarks/    # benchmark相关文件
├─ docs/          # 项目相关文档
├─ CMakeLists.txt
└─ README.md
```

## 主要 Targets

```text
oscillator_core
harmonic_oscillator
underdamped_oscillator
oscillator_batch
oscillator_benchmark
oscillator_tests
```

可视化程序会使用独立 target，避免窗口、绘制和帧率逻辑进入核心计算库或 benchmark 计时区间。

## 构建

从仓库根目录执行：

```bash
cmake -S . -B build
cmake --build build --config Release
```

单独构建测试和 benchmark：

```bash
cmake --build build --config Release --target oscillator_tests oscillator_benchmark
```

当前主要开发环境：

- C++20
- CMake 3.20+
- Visual Studio / MSVC x64
- Windows 10 / 11
