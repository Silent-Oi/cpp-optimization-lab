# Project02 — CPU/CUDA Heat Equation

> Two-dimensional heat diffusion, numerical verification, and CPU/GPU execution.

**前四阶段已完成。CPU 与 CUDA 差分求解器已通过数值对照，当前进入第五阶段的执行成本分析与优化。**

## 项目成果

实现二维热扩散模拟器，提供可信的 CPU 参考、CUDA 计算后端和温度场显示。两种后端使用相同的物理模型、初始条件和边界条件，比较数值结果与执行成本。

基础求解模型采用矩形规则网格、均匀材料、常数热扩散系数、无内部热源和固定温度边界。交互模式在内部节点额外施加局部温度增量，观察其后的扩散；验证场景另选与边界相容的解析参考。

本项目从 Project01 的独立振子更新进入有邻域依赖的网格计算，为后续稀疏 Poisson 求解器提供离散、边界处理和误差分析的基础。

## 技术路线与当前入口

当前实验从已有的 512 步求解计时接入，先测量 GPU 连续推进占整次求解的时间比例。实验起点与完成条件见[第五阶段](docs/roadmap.md#5-cpugpu-执行成本与瓶颈优化)。

已实现双线性温度插值、固定正负色标、RGFW 窗口、固定时间步推进及单节点温度交互。CPU 与 CUDA 实现统一接入 `heat_core`；构建本项目需要 CUDA Toolkit。

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

`heat_core` 支持均匀场、单正弦场初始化，以及 CPU 单线程、多线程和 CUDA 差分推进。一次 CUDA 求解在显存中交替使用两个温度缓冲区，完成指定步数后将最终温度场传回主机。`heat_tests` 的当前入口将 CUDA 结果与正弦模态解析解逐点比较。`heat_benchmark` 用于数值与性能实验，`heat_visualizer` 连续运行时使用 CUDA 求解，按 N 单步推进时使用 CPU 求解。

## 构建

项目已接入仓库根 CMake，共用 `build/<配置>`，不创建独立的嵌套构建目录。

Windows / MSVC x64 环境下，从仓库根目录执行：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build\x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build\x64-Release'
```

`heat_core` 是统一的静态库 target，包含 `src/heat_diffusion.cpp` 和 `cuda/cuda_heat_diffusion.cu`。CMake 分别用 C++ 与 CUDA 编译器编译，再归入同一个库；测试、benchmark 和显示程序均只需链接 `heat_core`。可通过 `cmake --build build/x64-Release --target heat_core` 单独构建；`heat_tests` 用于当前数值对照。这些目标不依赖 Project01。

Windows CUDA 工具链、安装组件和单个 `.cu` 文件的编译命令见 [CUDA 开发环境](docs/cuda_windows.md)。Visual Studio 中可选择 `x64-02-Release` 或 `x64-02-Debug` 配置单独构建 P2；`cuda_basics` 保留为独立的入门程序。

误差实验的绘图命令与输出说明见 [数值实验](benchmarks/README.md)。

## 运行与交互

在仓库根目录运行 `build/x64-Release/Project02_heat_equation/heat_visualizer.exe`。

| 操作 | 行为 |
|---|---|
| 空格 | 暂停／继续 |
| N | 暂停并推进一个物理时间步 |
| R | 恢复初始温度场并暂停 |
| 按住鼠标左键／右键 | 运行期间对鼠标对应的一个内部节点加热／冷却 |
| Esc | 退出 |

当前使用固定色标，零温度为中性色，正负温度分别趋向暖色和冷色；超出色标幅度的温度使用端点颜色。留白区与固定边界不接受温度输入，暂停期间不施加画笔。

画笔采用单位受热体积的简化约定，以本帧对应的模拟时间 `steps * dt` 计算温度增量，集中施加后再推进扩散。因此，同一段模拟时间按不同显示帧分组时，交互结果可能不同；当前模型不用于跨网格比较固定物理总功率。重置恢复温度场，但保留不足一个 `dt` 的计时余量。圆形笔刷、逐物理步热源和显示性能优化尚未实现。

## 完成后的能力

能够解释并实现一个带邻域依赖的 PDE 更新程序，建立解析参考与误差检查，定位基本数值和 GPU 执行问题，并区分 CPU/GPU 的计算、传输与整体成本。

后续衔接：[稀疏 Poisson 求解器与总路线](../docs/roadmap.md)。
