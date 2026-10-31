# Windows CUDA 开发环境

P2 统一构建 CPU 与 CUDA 实现，需要安装 CUDA Toolkit。`heat_core` 同时包含 `.cpp` 和 `.cu` 源文件，测试、benchmark 和显示程序统一链接该库。`cuda_basics` 是独立的入门目标，也可以直接使用 `nvcc` 编译。

## 已核对的本机配置

2026-10-31 配置：

| 项目 | 配置 |
|---|---|
| GPU | NVIDIA GeForce RTX 4070 Ti，12282 MiB 显存 |
| Compute Capability | 8.9，对应编译目标 `sm_89` |
| NVIDIA 驱动 | 621.01 |
| C++ 工具链 | Visual Studio Community 2026 18.11.3，MSVC 19.51.36260 x64 |
| CUDA Toolkit | 13.4.3，`nvcc` V13.4.92 |
| Toolkit 安装目录 | `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.4` |

NVIDIA 的 [Windows 安装指南](https://docs.nvidia.com/cuda/cuda-installation-guide-microsoft-windows/index.html#system-requirements)列出 CUDA 13.4 对 VS 2026／MSVC 195x 的支持。本机无需通过跳过编译器兼容检查来构建。

安装器来自 [NVIDIA 官方下载页](https://developer.nvidia.com/cuda-downloads)，安装前已验证 NVIDIA Corporation 数字签名。安装组件包括 `nvcc_13.4`、`nvvm_13.4`、`cudart_13.4`、`crt_13.4` 和 `visual_studio_integration_13.4`，所需的 CCCL 头文件由安装器依赖处理。未安装额外 GPU 驱动或性能分析工具。

`nvcc` 与 NVVM 在此版本中分别打包；只安装 `nvcc` 并不足以编译 GPU kernel。

## Visual Studio 编辑与构建

在配置下拉框中选择 `x64-02-Release` 或 `x64-02-Debug`，等待 CMake 配置完成，再选择 `heat_tests.exe`、`heat_visualizer.exe` 或 `cuda_basics.exe` 等启动目标。

启用 `CPP_LAB_BUILD_PROJECT02` 时，根 CMake 会启用 CUDA 编译支持。P2 不再单独提供 CUDA 开关；上述两套配置通过 `CPP_LAB_BUILD_PROJECT01=OFF` 排除 Project01。`x64-Debug` 和 `x64-Release` 默认包含两个项目，也需要 CUDA Toolkit。

命令行使用同一配置：

```powershell
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build\x64-02-Release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCPP_LAB_BUILD_PROJECT01=OFF -DCMAKE_CUDA_ARCHITECTURES=89 -DCMAKE_TRY_COMPILE_CONFIGURATION=Release && cmake --build build\x64-02-Release'
```

`89` 对应本机 GPU 的 Compute Capability 8.9；其他显卡应设置适合的架构。

`CMAKE_TRY_COMPILE_CONFIGURATION=Release` 让编译器探测使用 Release。若受限进程中出现 C1902 程序数据库管理器错误，可先在正常的 MSVC 开发环境中确认，避免误判为求解器错误。

`heat_core` 根据源文件语言分别设置编译选项：C++ 使用 MSVC 的 `/utf-8` 等选项，CUDA 使用 `nvcc` 的 `--utf-8`。根 CMake 还通过 `-Xcompiler` 为 CUDA 的主机编译阶段设置 UTF-8 与标准预处理器。

## 检查编译器

安装后重新启动终端或 IDE，使其读取更新后的 `PATH` 和 `CUDA_PATH`。PowerShell 中执行：

```powershell
nvcc --version
```

旧进程尚未刷新环境变量时，可直接使用完整路径：

```powershell
& 'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.4\bin\nvcc.exe' --version
```

`nvidia-smi` 显示的 CUDA 版本表示驱动支持能力；本机安装的编译工具版本以 `nvcc --version` 为准。

## 编译单个入门文件

以下命令用于直接编译 `Project02_heat_equation/cuda/cuda_basics.cu`。在仓库根目录执行：

```powershell
New-Item -ItemType Directory -Force build\x64-Release | Out-Null
cmd.exe /d /c 'call "D:\Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.4\bin\nvcc.exe" -std=c++20 -arch=sm_89 --utf-8 -Xcompiler=/utf-8,/Zc:preprocessor Project02_heat_equation\cuda\cuda_basics.cu -o build\x64-Release\cuda_basics.exe'
```

编译成功后运行：

```powershell
.\build\x64-Release\cuda_basics.exe
```

- `vcvars64.bat` 在同一 `cmd.exe` 进程中准备 MSVC 与 Windows SDK。
- `-std=c++20` 与仓库使用的 C++ 标准保持一致。
- `-arch=sm_89` 针对本机 GPU；其他显卡需要选择适合的目标。
- `--utf-8` 设置 CUDA 编译器的源文件与执行字符集。
- `-Xcompiler` 把选项传给 MSVC：`/utf-8` 指定字符编码，`/Zc:preprocessor` 启用标准预处理器。当前 CCCL 头文件要求标准预处理器。

## 环境验证范围

使用临时目录中的 [NVIDIA CUDA Samples](https://github.com/NVIDIA/cuda-samples) 验证环境。设备识别通过 `deviceQuery` 检查；显式显存分配、双向数据传输、kernel 计算与结果校验使用 [v13.0 的 vectorAdd](https://github.com/NVIDIA/cuda-samples/blob/v13.0/Samples/0_Introduction/vectorAdd/vectorAdd.cu)。官方样例未加入项目实现。环境验证不代表热扩散 GPU 后端已经实现或通过 CPU/GPU 数值对照。
