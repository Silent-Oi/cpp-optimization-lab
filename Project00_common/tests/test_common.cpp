#include <cstdlib>
#include <stdexcept>

namespace common {

void expect_near(double actual, double expected, double tolerance) {
    if (std::abs(actual - expected) <= tolerance) {
        return;
    }
    throw std::runtime_error("与预期结果不符");
}

}  // namespace common
