# Project00 — Common

Project00 是整个仓库的公共基础层，不是一个独立运行的项目。

它用于存放多个项目都会复用的基础组件，避免在每个项目里重复实现相同内容。

## 计划职责

初期可逐步加入：

- 计时工具
- 命令行参数读取
- 简单的结果校验工具
- 数值误差比较
- CSV 输出
- 随机数工具
- 基础向量或数学类型
- 编译器和平台信息输出

不要一开始把所有东西都放进 Common。只有当某个工具至少被两个项目需要时，再考虑移动到这里。

## 目录

```text
Project00_common/
├─ include/
│  └─ cpp_lab/          # 公共头文件建议放在该命名空间目录下
├─ src/                 # 只有需要单独编译的实现才放这里
├─ tests/
├─ benchmarks/
├─ CMakeLists.txt
└─ README.md
```

## 命名建议

公共代码建议使用命名空间：

```text
cpp_lab
```

头文件路径建议采用：

```text
Project00_common/include/cpp_lab/
```

未来示例：

```text
cpp_lab/timer.hpp
cpp_lab/numeric.hpp
cpp_lab/csv_writer.hpp
```

## 初期原则

1. 优先保持简单。
2. 不提前设计复杂框架。
3. 不加入和当前项目无关的工具。
4. 公共组件也必须有正确性测试。
5. 性能工具本身应尽量避免干扰被测代码。

## 当前状态

- [x] 建立目录结构
- [x] 建立 CMake 占位目标
- [ ] 添加计时工具
- [ ] 添加浮点数结果比较工具
- [ ] 添加基础测试
