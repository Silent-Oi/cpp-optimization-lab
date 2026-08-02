# C++ Optimization Lab

> A progressive C++ performance optimization lab for numerical computing and CPU graphics.

这是一个以实际程序为载体，逐步学习 **现代 C++、数值计算、CPU 性能优化与基础图形学** 的长期项目。

## 项目目标

### 中期目标：稀疏矩阵与高性能数值求解

实现并理解：

- CSR 稀疏矩阵存储
- Sparse Matrix-Vector Multiplication（SpMV）
- Conjugate Gradient（CG）
- 简单预条件
- Matrix-free operator
- 二维 Poisson 方程求解

这一阶段将数值算法、数据结构、内存访问与 CPU 性能分析连接起来。

### 最终目标：高性能 CPU 路径追踪器

完成一个包含以下能力的 CPU 路径追踪器：

- 三角形网格与材质系统
- 光线与几何求交
- Monte Carlo 随机采样
- BVH 空间加速结构
- 多线程并行渲染
- 缓存、数据布局与热点循环优化

这个项目将综合使用数学、物理、现代 C++、空间数据结构与 CPU 性能优化。

## 当前项目

### [Project01 — Batch Oscillator](Project01_batch_oscillator/README.md)

批量计算大量参数不同的欠阻尼振子，并将它们的状态绘制成相空间星云。

## 技术路线
### [Roadmap](docs/roadmap.md) ###

## 仓库结构

```text
cpp-optimization-lab/
├─ Project00_common/             # 公共组件入口
├─ Project01_batch_oscillator/   # 欠阻尼振子
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
