# Project02 — CPU/CUDA Heat Equation

> Two-dimensional heat diffusion, numerical verification, and CPU/GPU execution.

**状态：前两阶段已完成，具备 CPU 单线程与多线程差分推进、单正弦模态解析参考、数值误差实验和线程数对比结果。当前进入第三阶段：温度场显示与交互。**

## 项目成果

实现二维热扩散模拟器，提供可信的 CPU 参考、CUDA 计算后端和温度场显示。两种后端使用相同的物理模型、初始条件和边界条件，比较数值结果与执行成本。

初始范围为矩形规则网格、均匀材料、常数热扩散系数、无内部热源和固定温度边界。展示场景为板内热点向周围扩散、边界维持低温；验证场景另选与边界相容的解析参考。

本项目从 Project01 的独立振子更新进入有邻域依赖的网格计算，为后续稀疏 Poisson 求解器提供离散、边界处理和误差分析的基础。

## 技术路线与当前入口

六个阶段、完成边界和设计原则见 [技术路线](docs/roadmap.md)。

当前从温度到像素颜色的映射开始，接入窗口显示、固定时间步推进、暂停、单步与重置。基础显示完成后扩展局部温度交互；窗口输入、温度更新与绘制保持职责分离。随后引入 CUDA 后端；当前构建无需 CUDA Toolkit。

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

CPU 核心、`heat_tests` 和 `heat_benchmark` 已接入构建；当前支持均匀场、单正弦场初始化，以及单线程和多线程 CPU 差分推进。窗口显示与 CUDA 后端尚未实现。

## 构建

项目已接入仓库根 CMake，共用 `build/<配置>`，不创建独立的嵌套构建目录。

Windows / MSVC x64 环境下，从仓库根目录执行：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build\x64-Release'
```

当前 `heat_core` 是静态库 target，编译 `src/heat_diffusion.cpp`，可通过 `cmake --build build/x64-Release --target heat_core` 单独构建；`heat_tests` 用于当前数值对照。它们使用本项目与 `Project00_common` 的头文件路径，不依赖 Project01。后续添加示例入口；CUDA 配置在第四阶段接入。

误差实验的绘图命令与输出说明见 [数值实验](benchmarks/README.md)。

## 完成后的能力

能够解释并实现一个带邻域依赖的 PDE 更新程序，建立解析参考与误差检查，定位基本数值和 GPU 执行问题，并区分 CPU/GPU 的计算、传输与整体成本。

后续衔接：[稀疏 Poisson 求解器与总路线](../docs/roadmap.md)。
