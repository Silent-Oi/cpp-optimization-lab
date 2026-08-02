#include "test_common.h"
#include "state.h"

#include <sstream>
#include <stdexcept>
#include <string>



namespace oscillator {
const double tolerance = 1e-12;

void expect_near(double actual, double expected, const std::string& quantity) {
    if (std::abs(actual - expected) <= tolerance) {
        return;
    }

    std::ostringstream message;
    message.precision(17);
    message << quantity << ": expected " << expected << ", got " << actual;
    throw std::runtime_error(message.str());
}


void expect_state_near(const State& actual, const State& expected, const std::string& test_name) {
    expect_near(actual.position, expected.position, test_name + " position");
    expect_near(actual.velocity, expected.velocity, test_name + " velocity");
}

void expect_aos_near(const OscillatorAoS& actual, const OscillatorAoS& expected,
                           const std::string& test_name) {
    expect_near(actual.position, expected.position, test_name + " position");
    expect_near(actual.velocity, expected.velocity, test_name + " velocity");
    expect_near(actual.omega, expected.omega, test_name + " omega");
    expect_near(actual.zeta, expected.zeta, test_name + " zeta");
    expect_near(actual.m00, expected.m00, test_name + " m00");
    expect_near(actual.m01, expected.m01, test_name + " m01");
    expect_near(actual.m10, expected.m10, test_name + " m10");
    expect_near(actual.m11, expected.m11, test_name + " m11");
}

void expect_soa_near(const OscillatorSoABatch& actual, const OscillatorSoABatch& expected,
                     const std::string& test_name) {
    std::size_t N = actual.omega.size();
    for (std::size_t i = 0; i < N; ++i) {
        expect_near(actual.position[i], expected.position[i], test_name + " position");
        expect_near(actual.velocity[i], expected.velocity[i], test_name + " velocity");
        expect_near(actual.omega[i], expected.omega[i], test_name + " omega");
        expect_near(actual.zeta[i], expected.zeta[i], test_name + " zeta");
        expect_near(actual.m00[i], expected.m00[i], test_name + " m00");
        expect_near(actual.m01[i], expected.m01[i], test_name + " m01");
        expect_near(actual.m10[i], expected.m10[i], test_name + " m10");
        expect_near(actual.m11[i], expected.m11[i], test_name + " m11");
    }




}

}  // namespace oscillator
