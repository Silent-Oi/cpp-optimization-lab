#include "oscillator_batch.h"
#include "state.h"
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace oscillator {

void update_soa_batch_no_termination_parallel(OscillatorSoABatch_no_termination& soa_batch,
                                              int begin, int end, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    for (int i = 0; i < step; ++i) {
        update_soa_batch_step_no_termination(soa_batch, begin, end);
    };
}
}  // namespace oscillator
