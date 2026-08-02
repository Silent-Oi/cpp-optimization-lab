#pragma once
#include <vector>

#include "state.h"
#include "underdamped_oscillator.h"

namespace oscillator {

// vector 保证元素连续存储，作为后续数据布局实验的 AoS baseline。
using OscillatorBatch = std::vector<OscillatorAoS>;

// 使用固定 seed 初始化 number 个相互独立的振子，并为固定 dt 预计算各自的更新系数。
OscillatorBatch make_oscillator_aos_batch(int number, double dt, int seed);

// 原地推进一个时间步；只更新 position 和 velocity。
void update_aos_batch_step(OscillatorBatch& aos_batch);

// 对同一批状态连续执行 step 次单步更新。
void update_aos_batch(OscillatorBatch& aos_batch, int step);

// 汇总数量、校验值、状态幅值和有限性，供功能/压力运行记录结果。
BatchResults aos_batch_report(OscillatorBatch& aos_batch_updated);

OscillatorSoABatch make_oscillator_soa_batch(int number, double dt, int seed);

void update_soa_batch_step(OscillatorSoABatch& soa_batch);

void update_soa_batch(OscillatorSoABatch& soa_batch, int step);

BatchResults soa_batch_report(OscillatorSoABatch& soa_batch_updated);

}  // namespace oscillator
