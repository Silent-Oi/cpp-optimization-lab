#pragma once
#include <vector>

#include "state.h"
#include "underdamped_oscillator.h"

namespace oscillator {

using OscillatorBatch = std::vector<OscillatorAoS>;
OscillatorBatch make_OscillatorAoS_batch(int number, double dt, int seed);
void update_aos_batch_step(OscillatorBatch& aos_batch);
void update_aos_batch(OscillatorBatch& aos_batch, int step);
AoSResults aos_batch_report(OscillatorBatch& aos_batch_updated);
}  // namespace oscillator
