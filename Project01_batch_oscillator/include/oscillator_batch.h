#pragma once
#include <vector>

#include "state.h"
#include "underdamped_oscillator.h"

namespace oscillator {

// vector 保证元素连续存储。


// ============================================================================
// AOS振子相关
// ============================================================================

// ---- AOS振子创建 --------------------------------------------------------------

// 使用固定 seed 初始化 number 个相互独立的振子，并为固定 dt 预计算各自的更新系数。
OscillatorAoSBatch make_oscillator_aos_batch(int number, double dt, int seed);

// ---- AOS振子更新 --------------------------------------------------------------

// 原地推进一个时间步；只更新 position 和 velocity。
void update_aos_batch_step(OscillatorAoSBatch& aos_batch);

// 对同一批状态连续执行 step 次单步更新。
void update_aos_batch(OscillatorAoSBatch& aos_batch, int step);

// ---- 非对齐 AOS振子创建
// --------------------------------------------------------------

OscillatorAoSBatchWithPayload make_oscillator_aos_batch_with_payload(int number, double dt,
                                                                     int seed);

// ---- 非对齐 AOS振子更新
// --------------------------------------------------------------

void update_aos_batch_step_with_payload(OscillatorAoSBatchWithPayload& aos_batch);
void update_aos_batch_with_payload(OscillatorAoSBatchWithPayload& aos_batch, int step);

// ---- AOS振子输出相关信息 --------------------------------------------------------------

// 汇总数量、校验值、状态幅值和有限性，供功能/压力运行记录结果。
BatchResults aos_batch_report(OscillatorAoSBatch& aos_batch_updated);
BatchResults aos_batch_report(OscillatorAoSBatchWithPayload& aos_batch_updated);


// ============================================================================
// SOA振子相关
// ============================================================================

// ---- 带终止SOA振子创建 --------------------------------------------------------------

OscillatorSoABatch make_oscillator_soa_batch(int number, double dt, int seed);
OscillatorSoABatch make_oscillator_soa_batch(int number, double dt, int seed, double omega,
                                             double zeta);

// ---- 带终止SOA振子更新 --------------------------------------------------------------

void update_soa_batch_step(OscillatorSoABatch& soa_batch);
void update_soa_batch(OscillatorSoABatch& soa_batch, int step);

// ---- 无终止SOA振子更新 --------------------------------------------------------------

OscillatorSoABatch_no_termination make_oscillator_soa_batch_no_termination(int number, double dt,
                                                                           int seed);
OscillatorSoABatch_no_termination make_oscillator_soa_batch_no_termination(int number, double dt,
                                                                           int seed, double omega,
                                                                           double zeta);

// ---- 无终止SOA振子更新 --------------------------------------------------------------

void update_soa_batch_step_no_termination(OscillatorSoABatch_no_termination& soa_batch);
void update_soa_batch_no_termination(OscillatorSoABatch_no_termination& soa_batch, int step);

// ---- SOA振子输出相关信息 --------------------------------------------------------------

BatchResults soa_batch_report(OscillatorSoABatch& soa_batch_updated);
BatchResults soa_batch_report(OscillatorSoABatch_no_termination& soa_batch_updated);

}  // namespace oscillator
