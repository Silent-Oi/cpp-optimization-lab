# C++ Optimization Lab

> A progressive C++ performance optimization lab for numerical computing and CPU graphics.

这是一个以实际程序为载体，逐步学习 **现代 C++、数值计算、CPU 性能优化与基础图形学** 的长期项目。

## 项目目标

中期目标：稀疏矩阵与高性能数值求解

最终目标：高性能 CPU 路径追踪器

## 当前项目

### [Project01 — Batch Oscillator](Project01_batch_oscillator/README.md)

批量计算大量参数不同的欠阻尼振子。

## 技术路线

### [Roadmap](docs/roadmap.md) ###

## 完成项目

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
