# Project01 — Batch Oscillator 技术路线

> 百万级欠阻尼振子模拟、CPU 性能优化与相空间可视化。

Project01 研究同一个数值模型在不同数据布局、编译器优化和并行执行方式下的性能表现。

项目从正确的欠阻尼振子更新开始，将其扩展到百万级独立振子；随后围绕核心计算逐项优化，并使用统一 benchmark 对比每次改动；最终将振子状态绘制为实时相空间星云。

---

## 项目目标

Project01 包含两个相互连接的目标：

1. **在 CPU 上高效更新大量欠阻尼振子；**
2. **将振子状态实时绘制为相空间星云。**

完整流程为：

```text
物理模型
→ 正确的标量更新
→ 批量计算
→ 经测量验证的 CPU 优化
→ 实时可视化
```

---

## 核心模型

每个振子满足：

```text
x' = v
v' = -2 * zeta * omega * v - omega^2 * x
```

当振子参数和时间步 `dt` 固定时，一步更新写成：

```text
x_next = m00 * x + m01 * v
v_next = m10 * x + m11 * v
```

`m00`、`m01`、`m10` 和 `m11` 在初始化阶段预计算，核心热循环只负责重复更新状态。

可视化使用归一化相空间坐标：

```text
horizontal = position
vertical   = velocity / omega
```

---

# 性能优化路线

## 1. 正确的标量核心

在引入批量存储和性能优化前，先建立可信的单振子参考实现。

### 实现内容

- 欠阻尼解析状态；
- 固定时间步的精确一步更新系数；
- 单步和多步验证；
- 无阻尼回归情况；
- 非法参数与浮点异常检查。

### 结果

得到一个可信的状态更新核心，后续所有实现都以它作为正确性参考。

### 状态

**已完成**

---

## 2. AoS 批量基线

将单振子更新扩展为连续存储的大规模批量计算。

### 实现形式

```cpp
std::vector<OscillatorAoS>
```

基线版本包括：

- 使用固定随机种子进行可复现初始化；
- 保存每个振子的状态和参数；
- 预计算更新系数；
- 单线程批量更新；
- checksum 与有限值检查；
- 与标量参考结果对比。

### 结果

得到一个清晰、正确、刻意保持简单的性能基线。

### 状态

**已完成**

---

## 3. Benchmark Target

在加入优化版本前，建立可复用的独立 benchmark 可执行程序。

### Benchmark 职责

- 使用完全相同的输入运行多个实现；
- 将初始化、复制、结果汇总和文件输出排除在计时区间之外；
- 进行预热；
- 重复测量；
- 检查不同实现的结果一致性；
- 输出每个振子、每个时间步的更新时间；
- 输出 CSV；

主要指标：

```text
nanoseconds / oscillator / step
```

### 结果

得到能够重复用于 AoS、SoA、自动向量化和多线程对比的 benchmark target。

### 状态

**已完成**

---

## 4. 数据布局优化

第一项优化是将 Array of Structures 基线改为 Structure of Arrays。

### 基线布局

```text
OscillatorAoS
├─ position
├─ velocity
├─ omega
├─ zeta
├─ m00
├─ m01
├─ m10
└─ m11
```

### 优化布局

```text
position[]
velocity[]
m00[]
m01[]
m10[]
m11[]
omega[]
zeta[]
```

更新循环只访问状态数组和更新系数数组。热循环不需要的参数仍然保留，但不再进入核心数据流。

### 工作内容

- 实现 SoA 批量初始化；
- 实现 SoA 单步和多步更新；
- 验证 AoS 与 SoA 结果一致；
- 使用现有 benchmark 比较两种布局；
- 根据实测结果理解数据布局对性能的影响。

### 结果
完成项目的第一项正式优化：

```text
AoS baseline
→ SoA
→ benchmark 对比
```

### 状态

**已完成**

---

## 5. 编译器自动向量化

让编译器在条件允许时使用一条指令同时处理多个振子。

### 工作内容

- 检查当前 Release 版本；
- 开启并阅读向量化诊断；
- 必要时检查生成的核心循环；
- 找出阻碍向量化的数据依赖、指针别名或控制流；
- 在有依据时简化循环或接口；
- 使用现有 benchmark 对比优化前后。

主要目标是 SoA 更新循环：

```cpp
for (std::size_t i = 0; i < count; ++i)
{
    const double old_position = position[i];
    const double old_velocity = velocity[i];

    position[i] = m00[i] * old_position + m01[i] * old_velocity;
    velocity[i] = m10[i] * old_position + m11[i] * old_velocity;
}
```

只有当编译器因为明确原因无法生成理想代码时，才考虑手写 SIMD intrinsics。

### 结果

完成第二项正式优化：

```text
SoA
→ 自动向量化
→ benchmark 对比
```

### 状态

**计划中**

---

## 6. 多线程更新

将优化后的单线程核心按连续振子区间分配给多个 CPU 线程。

```text
thread 0 → [0, begin1)
thread 1 → [begin1, begin2)
thread 2 → [begin2, begin3)
...
```

振子彼此独立，因此每个线程可以无锁更新自己的区间。

### 工作内容

- 实现连续区间划分；
- 使用 `std::thread` 或 `std::jthread`；
- 验证单线程与多线程结果一致；
- 测试 1、2、4、8……线程；
- 计算加速比和并行效率；
- 找出继续增加线程不再带来有效收益的位置。

Project01 不实现通用线程池或任务调度系统。

### 结果

完成第三项正式优化：

```text
最佳单线程版本
→ 简单多线程
→ benchmark 对比
```

### 状态

**计划中**

---

# 可视化路线

可视化与性能优化并行推进，并与 benchmark 中的数值核心保持分离。

## 1. CPU 像素缓冲

建立线性 RGBA 图像缓冲：

```cpp
std::vector<std::uint32_t> pixels(width * height);
```

实现：

- 像素索引；
- 清空缓冲；
- 带边界检查的像素写入；
- 基础颜色编码。

### 结果

得到一张可以由 CPU 直接写入的图像缓冲。

---

## 2. 相空间坐标映射

将振子状态映射到屏幕坐标：

```text
phase_x = position
phase_y = velocity / omega
```

```cpp
pixel_x = center_x + phase_x * scale;
pixel_y = center_y - phase_y * scale;
```

处理：

- 平移与缩放；
- 屏幕 y 轴反向；
- 裁剪；
- 浮点坐标到整数像素的转换。

### 结果

振子状态能够以二维相空间点的形式显示。

---

## 3. 窗口与事件循环

使用 RGFW 作为轻量窗口层。

Visualizer 负责：

- 创建窗口；
- 处理事件；
- 显示像素缓冲；
- 处理关闭、暂停和重置输入；
- 持续更新与绘制。

主循环结构：

```cpp
while (window_is_open)
{
    process_input();
    update_simulation();
    draw_phase_space();
    present_pixels();
}
```

### 结果

得到能够实时显示振子星云的可执行程序。

---

## 4. 仿真时间与帧时间

物理更新使用固定时间步，不依赖显示刷新率。

每个渲染帧可以执行一个或多个固定仿真步，同时保持稳定的物理时间尺度。

### 结果

当帧率或绘制性能变化时，振子的物理演化速度保持一致。

---

## 5. 相空间星云效果

基础点云运行后，再加入少量有实际价值的显示效果：

- 按 `omega` 或 `zeta` 映射颜色；
- 短拖尾；
- 亮度累积；
- 暂停与重置；
- 缩放；
- 对超大规模数据进行抽样绘制或密度绘制。

本项目不需要通用 UI 框架、GPU 渲染管线或复杂场景系统。

### 结果

得到能够清晰展示旋转、失相和阻尼收缩的实时相空间星云。

---

# 最终整合

最终项目分为三个清晰部分：

```text
oscillator_core
├─ scalar reference
├─ AoS update
├─ SoA update
├─ vectorized update
└─ parallel update

oscillator_benchmark
├─ implementation comparison
├─ oscillator-count scaling
└─ thread-count scaling

oscillator_visualizer
├─ simulation backend
├─ phase-space mapping
├─ CPU pixel drawing
└─ RGFW window loop
```

Benchmark 只测量数值更新核心。

窗口、绘制、输入处理和帧时间均不进入性能计时区间。

---

# 完成标准

Project01 完成时应当能够：

1. 能够正确初始化并更新百万级欠阻尼振子；
2. 完成 AoS 与 SoA 实现及性能对比；
3. 完成 CPU 相空间星云可视化；
4. 确认并测试编译器自动向量化；
5. 完成简单多线程实现和线程扩展性测试；
6. 保证所有优化版本与参考实现结果一致；
7. 保存一组能够展示主要优化效果的最终 benchmark 结果。
