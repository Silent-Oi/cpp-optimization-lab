# Project02 — CPU/CUDA Heat Equation

> Two-dimensional heat diffusion, numerical verification, and CPU/GPU execution.

**状态：CPU 五点差分单步/多步更新及单正弦模态解析参考已实现，仍处于第一阶段的基础单线程实现整理。第二阶段将开展误差实验、数据访问理论分析与多线程对比。**

## 项目成果

实现二维热扩散模拟器，提供可信的 CPU 参考、CUDA 计算后端和温度场显示。两种后端使用相同的物理模型、初始条件和边界条件，比较数值结果与执行成本。

初始范围为矩形规则网格、均匀材料、常数热扩散系数、无内部热源和固定温度边界。展示场景为板内热点向周围扩散、边界维持低温；验证场景另选与边界相容的解析参考。

本项目从 Project01 的独立振子更新进入有邻域依赖的网格计算，为后续稀疏 Poisson 求解器提供离散、边界处理和误差分析的基础。

## 技术路线与当前入口

六个阶段、完成边界和设计原则见 [技术路线](docs/roadmap.md)。

当前完善单线程求解器的状态管理、时间层存储、计算与数据访问组织，并检查编译器生成代码。第二阶段分析离散误差、工作集与数据复用，实现并评估多线程推进。随后加入温度场显示和 CUDA 后端；当前构建无需 CUDA Toolkit。

## 项目结构

```text
Project02_heat_equation/
├─ include/       # 网格、参数和计算接口
├─ src/           # CPU 数值实现
├─ apps/          # 无窗口的运行示例
├─ tests/         # 随实现增加的必要正确性对照
├─ benchmarks/    # 性能阶段的测量与绘图
├─ visualizer/    # 温度场显示
├─ cuda/          # 第四阶段的 GPU 后端
├─ results/       # 本地生成结果，默认不跟踪
├─ docs/
│  ├─ learning_logs/ # 个人学习记录
│  └─ roadmap.md
├─ CMakeLists.txt
└─ README.md
```

CPU 核心及 `heat_tests` 已接入构建；当前支持均匀场、单正弦场初始化和 CPU 差分推进。窗口显示与 CUDA 后端尚未实现。

## 构建

项目已接入仓库根 CMake，共用 `build/<配置>`，不创建独立的嵌套构建目录。

Windows / MSVC x64 环境下，从仓库根目录执行：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build\x64-Release'
```

当前 `heat_core` 是静态库 target，编译 `src/heat_diffusion.cpp`，可通过 `cmake --build build/x64-Release --target heat_core` 单独构建；`heat_tests` 用于当前数值对照。它们使用本项目与 `Project00_common` 的头文件路径，不依赖 Project01。后续添加示例入口；CUDA 配置在第四阶段接入。

## 完成后的能力

能够解释并实现一个带邻域依赖的 PDE 更新程序，建立解析参考与误差检查，定位基本数值和 GPU 执行问题，并区分 CPU/GPU 的计算、传输与整体成本。

后续衔接：[稀疏 Poisson 求解器与总路线](../docs/roadmap.md)。
