cmake_minimum_required(VERSION 3.20)

project(
    cpp_optimization_lab
    VERSION 0.1.0
    DESCRIPTION "Progressive C++ optimization projects for numerical and physical computing"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CPP_LAB_BUILD_TESTS "Build project tests" ON)
option(CPP_LAB_BUILD_BENCHMARKS "Build project benchmarks" ON)

add_subdirectory(Project00_common)
add_subdirectory(Project01_batch_oscillator)
