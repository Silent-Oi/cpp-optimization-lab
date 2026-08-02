# NOW — Project01 / AoS Benchmark

打开仓库后先看这里，然后直接写代码。这里只有一个活动任务。

## 现在打开

```text
Project01_batch_oscillator/benchmarks/aos_batch_benchmark.cpp
```

## 当前目标

完成一个不会把时间浪费在无意义超长运行上的 AoS benchmark，观察不同数据规模下的更新吞吐量。

## 现在写

1. 不再让所有规模固定运行 `1000` 步。
2. 让每个规模的 `N × steps` 保持在相近数量级，或用一次短校准决定 `steps`。
3. 保留预热、相同初始状态、7 次重复、checksum 和中位数输出。
4. Release 运行后，先看吞吐量随 `N` 的变化，不写实验报告。

## 这一步完成的标志

一次运行能在合理时间内结束，并输出多个 `N` 的稳定结果：

```text
N
steps
total_updates
median_seconds
updates_per_second
nanoseconds_per_update
checksum
```

## 完成后

进入 SoA 实现，并与同一组 AoS 输入进行比较。

## 现在不要做

- 不整理实验报告。
- 不写 CSV 框架。
- 不抽象通用 benchmark 库。
- 不提前做多线程。
- 不动相空间星云；它在 AoS/SoA 主线结束后连续完成。
