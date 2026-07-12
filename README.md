# C++ Optimization Lab

这是一个以 **C++ 性能优化、数值计算和物理问题** 为主线的渐进式练习仓库。

当前阶段只建立仓库基础和前两个项目：

- **Project00 — Common**：公共基础设施、通用工具和仓库规范。
- **Project01 — Batch Oscillator**：批量简谐振子模拟，作为第一个性能优化实验。

Project00 不是一个独立的物理项目，而是后续所有项目共享的基础层。

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
│  ├─ include/               # Project01 头文件
│  ├─ src/                   # Project01 源文件
│  ├─ tests/                 # 正确性测试
│  ├─ benchmarks/            # 性能测试
│  ├─ docs/                  # 设计与实验计划
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
├─ .gitignore
└─ CMakeLists.txt
```

## 项目目标

这套仓库不是为了堆很多功能，而是为了逐步回答下面这些问题：

1. C++ 的数据布局为什么会影响性能？
2. 连续内存、缓存局部性和循环顺序有什么关系？
3. 如何区分“代码更复杂”和“代码真的更快”？
4. 如何建立可复现的性能测试？
5. 如何在保证数值正确性的前提下进行优化？

## 环境

推荐环境：

- C++17
- CMake 3.20+
- Visual Studio 2026
- Windows 10 / 11

当前仓库不要求第三方库。

## 初始化构建目录

在仓库根目录执行：

```bash
cmake -S . -B build
cmake --build build --config Release
```

目前仓库只有项目骨架，没有可执行程序。添加源码和目标后才会产生可执行文件。

## 建议的 Git 工作流

完成一个小阶段后提交一次：

```bash
git status
git diff
git add .
git commit -m "project01: add initial oscillator model"
```

提交信息建议使用：

```text
project00: ...
project01: ...
docs: ...
build: ...
test: ...
benchmark: ...
refactor: ...
```

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

## 当前进度

- [x] 建立仓库基础结构
- [x] 建立 Project00 Common 骨架
- [x] 建立 Project01 Batch Oscillator 骨架
- [ ] 添加 Project00 第一批公共组件
- [ ] 完成 Project01 正确性基线
- [ ] 完成 Project01 第一次性能对比
