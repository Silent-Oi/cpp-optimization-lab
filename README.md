# C++ Optimization Lab

> A project-based C++ lab for numerical computing, CPU/GPU performance, and graphics.

这是一个以实际程序为载体，逐步学习 **现代 C++、数值计算、CPU/GPU 性能优化与图形学** 的长期项目。

## 能力目标

本仓库围绕两个方向展开：

- **稀疏矩阵与高性能计算**：数值方法、稀疏数据结构、迭代求解、内存访问与 CPU/GPU 并行计算。
- **路径追踪与图形渲染**：几何求交、光照与采样、图像生成、空间加速结构与并行渲染。

以稀疏 Poisson 求解器作为中期作品，以高性能 CPU 路径追踪器作为最终整合作品，逐步建立独立实现、解释和扩展程序的能力。

## 当前项目

### [Project01 — Batch Oscillator](Project01_batch_oscillator/README.md)

批量计算大量参数不同的欠阻尼振子。

下一项目为 [Project02 — CPU/CUDA Heat Equation](Project02_heat_equation/README.md)，目前处于规划阶段。先实现并验证 CPU 热扩散模拟，再引入 CUDA；CUDA 不属于当前 Project01 的构建依赖。

## 技术路线

[查看技术路线](docs/roadmap.md)

## 仓库结构

```text
cpp-optimization-lab/
├─ Project00_common/             # 公共组件入口
├─ Project01_batch_oscillator/   # 欠阻尼振子
├─ Project02_heat_equation/     # CPU/CUDA 热扩散（规划中）
├─ docs/
│  └─ roadmap.md                 # 总体技术路线
├─ CMakeLists.txt
└─ CMakeSettings.json
```

每个子项目通常包含：

```text
ProjectXX/
├─ include/       # 公共接口
├─ src/           # 核心实现
├─ apps/          # 可执行程序
├─ tests/         # 正确性测试
├─ benchmarks/    # 性能测试
├─ docs/          # 项目相关文档
├─ visualizer/    # 可视化
└─ CMakeLists.txt
```

## 构建

项目使用 C++20 和 CMake。

```bash
cmake -S . -B build
cmake --build build --config Release
```

当前主要开发环境：

- C++20
- CMake 3.20+
- Visual Studio / MSVC x64
- Windows 10 / 11

## 项目状态

项目仍在持续开发中。实现、接口与目录结构会随着技术路线推进而演化。
