#include <cstdlib>
#include <stdexcept>
namespace {

const double tolerance = 1e-4;

}

namespace common {

void expect_near(double actual, double expected) {
    if (std::abs(actual - expected) <= tolerance) {
        return;
    }
    throw std::runtime_error("与预期结果不符");
}

}  // namespace common
