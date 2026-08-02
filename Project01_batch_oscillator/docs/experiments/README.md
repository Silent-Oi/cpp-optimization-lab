# Project01 实验报告

本目录保存可以提交 Git、能够复现实验条件和结论的 Markdown 报告。

使用仓库根目录的 [`benchmark_record_template.md`](../../../docs/benchmark_record_template.md)
创建报告。建议文件名包含里程碑或日期和唯一实验因素。

大体积原始计时输出、CSV 和本地临时结果保存在项目 `results/`，不要只在原始文件中
保留唯一一份实验结论。

## 当前报告

- [AoSBatch 振子计算步数与输入数据规模关系实验](<AoSBatch 振子计算时间步与数据量规模的关系.md>)
  - 固定 `N = 256`、改变 `steps` 的诊断实验；它不是 M3 AoS baseline 的多规模正式报告。
