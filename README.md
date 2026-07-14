# C++ Optimization Lab

这是一个以 **C++ 性能优化、数值计算和物理问题** 为主线的渐进式练习仓库。

当前阶段：

- **Project00 — Common**：公共基础设施、通用工具和仓库规范。
- **Project01 — Batch Oscillator**：批量简谐振子模拟，作为第一个性能优化实验。


## 仓库结构

```text
cpp-optimization-lab/
├─ Project00_common/
│  ├─ include/               # 公共头文件
│  ├─ src/                   # 必要时存放公共实现
│  ├─ tests/                 # 公共组件测试
│  ├─ benchmarks/            # 公共组件基准测试
│  ├─ CMakeLists.txt
│  └─ README.md
│
├─ Project01_batch_oscillator/
│  ├─ include/
│  │  ├─ state.h
│  │  ├─ harmonic_oscillator.h
│  │  └─ explicit_euler_step.h
│  ├─ src/
│  │  ├─ main.cpp
│  │  ├─ harmonic_oscillator.cpp
│  │  └─ explicit_euler_step.cpp
│  ├─ tests/                 # 正确性测试
│  ├─ benchmarks/            # 性能测试
│  ├─ docs/
│  │  ├─ design.md
│  │  ├─ experiment_plan.md
│  │  └─ progress_checklist.md
│  ├─ results/               # 本地实验结果，默认不提交
│  ├─ CMakeLists.txt
│  └─ README.md
│
├─ docs/
│  ├─ roadmap.md
│  ├─ benchmark_record_template.md
│  └─ learning_log_template.md
│
├─ .clang-format
├─ .editorconfig
├─ .gitattributes            # Git 文本文件与行尾规则
├─ .gitignore
├─ AGENTS.md                 # Codex 审阅与仓库维护约定
├─ CMakeSettings.json        # Visual Studio 的 Debug / Release 配置
└─ CMakeLists.txt
```

## 项目目标

这套仓库为了逐步理解：

1. C++ 的数据布局为什么会影响性能？
2. 连续内存、缓存局部性和循环顺序有什么关系？
3. 如何区分“代码更复杂”和“代码真的更快”？
4. 如何建立可复现的性能测试？
5. 如何在保证数值正确性的前提下进行优化？

## 环境

推荐环境：

- C++20
- CMake 3.20+
- Visual Studio 2026
- Windows 10 / 11

当前仓库不要求第三方库。


## 实验原则

进行性能实验时：

- 优先使用 Release 构建。
- 每次只改变一个主要因素。
- 同时检查运行时间和计算结果。
- 至少重复运行数次。
- 记录编译器、参数、规模和硬件环境。
- 不根据一次偶然结果下结论。

实验记录模板见：

- [`docs/benchmark_record_template.md`](docs/benchmark_record_template.md)
- [`docs/learning_log_template.md`](docs/learning_log_template.md)
