# Project01 — Batch Oscillator Roadmap

## 项目目标

Project01 使用百万级欠阻尼振子，完成一次完整的：

```text
正确计算
→ 批量数据组织
→ CPU 性能分析与优化
→ 实时可视化
```

实现效果：

1. 高效批量更新大量欠阻尼振子；
2. 将振子状态实时绘制成相空间星云。

---

## 核心计算

欠阻尼振子的状态为：

```text
x' = v
v' = -2 * zeta * omega * v - omega^2 * x
```

在参数和固定时间步 `dt` 不变时，每一步更新写成：

```text
x_next = m00 * x + m01 * v
v_next = m10 * x + m11 * v
```

更新系数 `m00`、`m01`、`m10`、`m11` 在初始化阶段预计算，热循环只负责状态更新。

相空间映射使用：

```text
horizontal = position
vertical   = velocity / omega
```

---

# 技术路线

## 1. 单振子与正确性基线

## 目标

- 谐振子数值方法实验；
- 欠阻尼解析状态；
- 固定 `dt` 的精确一步更新系数；
- 单步、多步与无阻尼回归测试；
- 非法参数和浮点异常处理。

### 学习内容

- 二阶微分方程与状态空间；
- 欠阻尼解析解；
- 精确一步更新；
- 预计算与热循环；
- 浮点数正确性；
- 回归测试。

### 问题

> 计算出的振子状态是否可信？

### 状态

已完成。

---

## 2. AoS 批量计算

### 目标

- 使用 `std::vector<OscillatorAoS>` 连续保存大量振子；
- 固定随机种子，生成可复现输入；
- 批量更新所有振子；
- 与单振子参考结果对比；
- 使用 checksum 防止计算被优化掉，并检查大规模结果。

### 学习内容

- `struct`；
- `std::vector`；
- 连续内存；
- 批量初始化；
- 数据生命周期；
- 核心算法与程序入口分离。

### 问题

> 怎样实现百万级振子同时更新时？

### 状态

已完成。

---

## 3. Benchmark 与存储层级

### 目标

固定：

```text
seed
dt
steps
repeat count
```

改变：

```text
oscillator count N
```

测量：

```text
ns / oscillator / step
```

只把批量更新放入计时区间，初始化、复制、输出和 checksum 均放在计时区间之外。

### 学习内容

- Release 与 Debug；
- `std::chrono::steady_clock`；
- benchmark 计时边界；
- 重复测量；
- 工作集大小；
- cache line；
- L1、L2、L3 与主内存；
- 硬件预取；
- 内存带宽。

### 问题

> 为什么同一段代码只因为数据规模变大，就会突然变慢？

### 状态

进行中

---

## 4. SoA 与数据布局

### 目标

- 使用独立连续数组保存各字段；
- 保持物理模型、输入和更新算法不变；
- 验证 AoS 与 SoA 计算结果一致；
- 在相同实验条件下比较 AoS 与 SoA；
- 分析两种布局的工作集大小和缓存拐点。

### 学习内容

- AoS 与 SoA；
- 核心循环实际使用的字段；
- cache line 利用率；
- 连续数据流；
- 内存流量；
- 数据布局与访问方式的关系。

### 问题

- AoS 每更新一个振子实际搬运多少字节？
- SoA 每更新一个振子实际搬运多少字节？
- 哪些字段只在初始化阶段使用？
- 哪些字段在每一步都需要？
- 两种布局的缓存拐点为什么不同？
- SoA 的性能变化来自缓存、带宽还是 SIMD 条件？

> CPU 需要的数据应该以什么形式放进内存？

### 状态

进行中。

---

## 5. CPU 可视化基础

### 5.1 CPU 像素缓冲

建立线性像素数组：

```cpp
std::vector<std::uint32_t> pixels(width * height);
```

实现：

- 清屏；
- 写一个像素；
- 越界检查；
- RGBA 颜色；
- 二维图像的线性存储。

像素索引：

```cpp
index = y * width + x;
```

### 问题

> 二维图像在 CPU 内存中是什么？

---

### 5.2 相空间坐标映射

将：

```text
phase_x = position
phase_y = velocity / omega
```

映射到：

```text
pixel_x
pixel_y
```

典型形式：

```cpp
pixel_x = center_x + phase_x * scale;
pixel_y = center_y - phase_y * scale;
```

需要处理：

- 原点平移；
- 缩放；
- 屏幕 y 轴向下；
- 浮点坐标转整数像素；
- 范围裁剪。

### 问题

> 内存里的数值怎样变成屏幕上的位置？

---

### 5.3 窗口与事件循环

使用RGFW窗口层完成：

- 创建窗口；
- 接收输入；
- 显示像素缓冲；
- 检测关闭事件。

主循环保持简单：

```cpp
while (window_is_open)
{
    process_input();
    update_simulation();
    clear_pixels();
    draw_oscillators();
    present_pixels();
}
```

### 学习内容

- 实时程序主循环；
- 帧；
- 输入事件；
- 窗口刷新；
- CPU 计算与显示。

---

### 5.4 仿真时间与显示时间分离

物理更新使用固定：

```text
simulation_dt
```

显示按实际帧率刷新。

初始实现可以每帧执行固定数量的 simulation steps，再绘制一次；之后再理解 accumulator 固定时间步。

### 问题

> 为什么仿真速度不应该依赖显示器帧率？

---

### 5.5 星云效果

基础点云运行后，再加入：

- 按 `omega` 或 `zeta` 映射颜色；
- 短拖尾；
- 像素亮度累积；
- 暂停；
- 重置；
- 显示缩放。

### 状态

进行中。

---

## 6. 编译器自动向量化

### 目标

围绕核心 SoA 循环检查：

```cpp
for (std::size_t i = 0; i < count; ++i)
{
    const double old_x = x[i];
    const double old_v = v[i];

    x[i] = a[i] * old_x + b[i] * old_v;
    v[i] = c[i] * old_x + d[i] * old_v;
}
```

确认：

- 编译器是否一次处理多个 `double`；
- AoS 和 SoA 哪个更容易向量化；
- 循环写法是否阻碍向量化；
- 编译器报告与实际汇编是否一致；
- 性能变化来自 SIMD 还是内存布局。

### 学习内容

- SIMD；
- 标量指令与向量指令；
- 自动向量化；
- 循环依赖；
- 指针别名；
- 连续内存；
- 对齐；
- 编译器优化报告；
- 核心循环汇编。

### 手写 SIMD（可选）

- 编译器没有生成理想代码；
- 已经知道具体阻碍；
- 手写版本能够回答明确的性能问题。

### 问题

> 一个 CPU 核心怎样同时处理多个振子？

### 状态

待实现。

---

## 7. 简单多线程

### 并行方式

将振子数组划分为连续区间：

```text
thread 0: [0,       N/4)
thread 1: [N/4,     N/2)
thread 2: [N/2,   3*N/4)
thread 3: [3*N/4,     N)
```

每个线程：

- 只处理自己的区间；
- 不使用锁；
- 不写入其他线程的数据；
- 完成后由主线程等待。

### 目标

- 使用 `std::thread` 或 `std::jthread`；
- 连续区间分块；
- 线程创建与 `join`；
- 单线程与多线程结果一致性检查；
- 比较不同线程数下的性能。

线程数测试：

```text
1
2
4
8
...
```

加速比：

```text
speedup = single_thread_time / parallel_time
```

### 学习内容

- 线程创建；
- lambda 捕获；
- `join`；
- 数据竞争；
- false sharing；
- 硬件线程；
- 并行加速比；
- 内存带宽饱和。

### 问题

- 小规模为什么多线程反而更慢？
- 为什么加速比通常小于线程数？
- 为什么线程增加到一定数量后不再加速？
- AoS 与 SoA 的并行饱和点是否不同？
- 当前瓶颈是计算能力还是内存带宽？
> 多个 CPU 核心怎样共同处理同一个大数组？

### Benchmark 与可视化的区别

纯 benchmark 中，各线程可以在自己的区间连续执行全部 `steps`。

实时可视化中，每帧必须等待所有线程更新完成，再开始绘制统一时刻的状态。

### 状态

待实现。

---

## 8. 整合

```text
oscillator_core
├─ scalar/reference
├─ AoS
├─ SoA
└─ parallel update

oscillator_benchmark
├─ layout comparison
├─ vectorization comparison
└─ thread scaling

oscillator_visualizer
├─ simulation backend
├─ phase-space mapping
├─ pixel drawing
└─ window loop
```
---

# 完成标准

Project01 完成时，应当能够：

1. 正确初始化和更新百万级欠阻尼振子；
2. 解释数据规模跨越 L1、L2、L3 和主内存时的性能变化；
3. 解释 AoS 与 SoA 的性能差异；
4. 确认并理解编译器自动向量化；
5. 使用简单多线程并解释加速饱和；
6. 使用 CPU 像素缓冲实时绘制相空间星云；
7. 保证优化版本与参考版本结果一致。
