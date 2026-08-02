# Project00 — Common

> Shared C++ components used across the optimization projects.

`Project00_common` 是 **C++ Optimization Lab** 的公共基础层，用于存放多个子项目真正需要复用的轻量组件。

## 作用

随着项目推进，可能进入 Common 的内容包括：

- 基础数学类型
- 计时工具
- 对齐存储辅助
- 简单图像或文件输出工具
- 公共编译配置
- 小型测试辅助函数

项目专属的数据结构、算法和业务逻辑仍然保留在各自目录中。

## 当前状态

Project00 目前提供一个 CMake `INTERFACE` target：

```cmake
cpp_lab_common
```

它负责：

- 暴露公共头文件目录
- 统一要求 C++20
- 为后续公共组件提供稳定的依赖入口

当前没有需要编译的公共源文件，也没有独立可执行程序。

## 使用方式

子项目可以通过 CMake 链接公共 target：

```cmake
target_link_libraries(
    your_target
    PRIVATE
        cpp_lab_common
)
```

随后即可使用 `Project00_common/include/` 中的公共头文件。

## 目录结构

```text
Project00_common/
├─ include/       # 公共头文件
├─ CMakeLists.txt
└─ README.md
```

当未来出现真正共享的实现代码时，可以再加入 `src/` 和编译型 library target。

## 设计原则

一个组件进入 Project00 前，应满足：

1. 已经在至少两个子项目中出现真实复用需求；
2. 接口经过实际使用，职责已经基本明确；
3. 提取后能够减少重复，而不会隐藏子项目的核心逻辑。

Project00 会保持小而稳定，只承载已经被项目证明有价值的公共能力。
