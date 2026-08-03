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

[`docs/roadmap.md`](docs/roadmap.md)

## 项目结构

```text
Project01_batch_oscillator/
├─ include/       # 模型与公共批量接口
├─ src/           # 核心振子实现
├─ apps/          # 功能示例入口
├─ tests/         # 正确性与回归测试
├─ benchmarks/    # benchmark相关文件
├─ docs/          # 项目相关文档
├─ visualizer/    # 可视化
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
oscillator_visualizer
```

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
