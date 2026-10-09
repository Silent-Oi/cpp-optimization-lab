# Project02 — CPU/CUDA Heat Equation

> Two-dimensional heat diffusion, numerical verification, and CPU/GPU execution.

**状态：2026-10-09 完成项目初始化，进入第一阶段。数值核心尚未实现。**

## 项目成果

实现二维热扩散模拟器，提供可信的 CPU 参考、CUDA 计算后端和温度场显示。两种后端使用相同的物理模型、初始条件和边界条件，比较数值结果与执行成本。

初始范围为矩形规则网格、均匀材料、常数热扩散系数、无内部热源和固定温度边界。展示场景为板内热点向周围扩散、边界维持低温；验证场景另选与边界相容的解析参考。

本项目从 Project01 的独立振子更新进入有邻域依赖的网格计算，为后续稀疏 Poisson 求解器提供离散、边界处理和误差分析的基础。

## 技术路线与当前入口

六个阶段、完成边界和设计原则见 [技术路线](docs/roadmap.md)。

当前从热方程的物理含义、一个简单解析解和有限差分更新入手，再实现小型 CPU 网格的一步与多步推进。CPU 结果稳定后进入显示和 CUDA；现阶段无需 CUDA Toolkit。

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

目前上述实现目录只有占位文件，尚无计算程序、测试程序或 CUDA kernel。

## 构建

项目已接入仓库根 CMake，共用 `build/<配置>`，不创建独立的嵌套构建目录。

Windows / MSVC x64 环境下，从仓库根目录执行：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build\x64-Release'
```

当前 `heat_core` 是 `INTERFACE` target，仅提供头文件路径、C++20 公共依赖与 MSVC UTF-8 选项，不产生库文件或可执行程序，也不是可单独构建的编译目标。第一份 CPU 实现加入时，再将其转为编译型库并添加示例入口。CUDA 配置在第四阶段接入。

## 完成后的能力

能够解释并实现一个带邻域依赖的 PDE 更新程序，建立解析参考与误差检查，定位基本数值和 GPU 执行问题，并区分 CPU/GPU 的计算、传输与整体成本。

后续衔接：[稀疏 Poisson 求解器与总路线](../docs/roadmap.md)。
