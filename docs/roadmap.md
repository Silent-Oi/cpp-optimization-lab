# 技术路线

- **中期：稀疏矩阵与高性能数值求解器**
- **最终：高性能 CPU 路径追踪器**

## 路线总览（暂定）

| 项目 | 目标 | 主要新增能力 | 通向下一步的作用 |
|---|---|---|---|
| Project01 — Batch Oscillator | 百万级欠阻尼振子更新与相空间星云 | 连续内存、AoS/SoA、benchmark、自动向量化、CPU 像素绘制 | 建立第一个完整的计算—优化—显示闭环 |
| Project02 — 2D Heat Equation | 二维热扩散模拟与温度场显示 | 规则网格、stencil、边界条件、稳定性、二维内存局部性 | 从独立元素更新进入具有邻域依赖的 PDE 计算 |
| Project03 — Sparse Poisson Solver | CSR / matrix-free Poisson 求解器 | 稀疏矩阵、SpMV、CG、预条件、残差与收敛 | **达到中期目标** |
| Project04 — Basic CPU Ray Tracer | 能输出图像的基础 CPU 光线追踪器 | 向量几何、相机、光线求交、材质、图像缓冲 | 建立最终项目的正确渲染基线 |
| Project05 — Monte Carlo Rendering | 可渐进收敛的随机采样渲染器 | RNG、统计误差、采样、累积缓冲、并行独立任务 | 从确定性光追进入路径追踪 |
| Project06 — Spatial Hierarchy | BVH；若跨度过大，可先做 Barnes–Hut 过渡 | 包围盒、树构建、树遍历、空间层次、数据布局 | 解决复杂场景的求交规模问题 |
| Project07 — CPU Path Tracer | 多线程高性能 CPU 路径追踪器 | 三角形网格、BVH、随机反弹、任务调度、综合性能分析 | **达到最终目标** |

## 第一段：通向稀疏矩阵

### Project01 — 批量欠阻尼振子

学习大量独立状态的连续更新：数据怎么放、CPU 怎么读、编译器能否向量化。以及数据怎么画出来。
[Project01 — roadmap.md](../Project01_batch_oscillator/docs/roadmap.md)


### Project02 — 二维热方程

把“每个元素只更新自己”推进到“每个网格点依赖邻居”。这里学习 stencil、边界条件、稳定性和二维数组的线性存储。

### Project03 — 稀疏 Poisson 求解器

把规则网格离散写成线性系统，完成 CSR、SpMV、共轭梯度法、简单预条件和 matrix-free 版本。该项目完成即达到中期目标。

## 第二段：通向 CPU 路径追踪

### Project04 — 基础 CPU 光线追踪器

先做正确图像，不急着优化。建立相机、光线、几何求交、材质和图像输出。

### Project05 — Monte Carlo 渲染

加入随机采样、渐进累积和多次反弹所需的统计基础，并学习独立像素任务的并行方式。

### Project06 — 空间层次结构

直接实现 BVH。只有当树结构跨度对当时能力过大时，才插入 Barnes–Hut 作为过渡项目；它不是固定必修课。

### Project07 — 高性能 CPU 路径追踪器

整合三角形网格、BVH、随机采样、多线程和性能分析，完成最终作品。
