add_library(cpp_lab_common STATIC
    benchmarks/common_benchmark.cpp
    tests/test_common.cpp
 "tests/test_common.h" "benchmarks/common_benchmark.h")

target_include_directories(
    cpp_lab_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks
)

if(MSVC)
    target_compile_options(cpp_lab_common PRIVATE /utf-8)
endif()
