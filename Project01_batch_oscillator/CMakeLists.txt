add_library(oscillator_core
    src/harmonic_oscillator.cpp
    src/oscillator_batch.cpp
    src/time_integrators.cpp
    src/underdamped_oscillator.cpp
)

target_include_directories(oscillator_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(oscillator_core
    PUBLIC
        cpp_lab_common
)

target_compile_definitions(
    oscillator_core
    PUBLIC
        PROJECT01_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# 谐振子实验
add_executable(harmonic_oscillator
    apps/harmonic_oscillator_main.cpp
)

target_link_libraries(harmonic_oscillator
    PRIVATE oscillator_core
)

# 欠阻尼振子实验
add_executable(underdamped_oscillator
    apps/underdamped_oscillator_main.cpp
)

target_link_libraries(underdamped_oscillator
    PRIVATE oscillator_core
)

# AoS 批量功能与压力实验
add_executable(oscillator_batch
    apps/oscillator_batch_main.cpp
)

target_link_libraries(oscillator_batch
    PRIVATE oscillator_core
)

# benchmark 独立于功能示例和测试，避免输出与计时代码进入 oscillator_core。
add_executable(oscillator_benchmark
    "benchmarks/batch_benchmark.cpp"
)

target_link_libraries(oscillator_benchmark
    PRIVATE oscillator_core
)


add_executable(oscillator_tests
    tests/test_common.cpp
    tests/test_common.h
    tests/test_harmonic_oscillator.cpp
    tests/test_harmonic_oscillator.h
    tests/test_main.cpp
    tests/test_oscillator_batch.cpp
    tests/test_oscillator_batch.h
    tests/test_underdamped_oscillator.cpp
    tests/test_underdamped_oscillator.h
)

target_include_directories(oscillator_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(oscillator_tests
    PRIVATE oscillator_core
)

add_executable(oscillator_visualizer
    visualizer/main.cpp
 "visualizer/pixel_buffer.h" "visualizer/pixel_buffer.cpp" "visualizer/rgfw_impl.cpp" "visualizer/phase_space_renderer.cpp" "visualizer/phase_space_renderer.h")

target_include_directories(oscillator_visualizer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/visualizer
        ${CMAKE_SOURCE_DIR}/third_party/RGFW
)

target_link_libraries(oscillator_visualizer
    PRIVATE oscillator_core
)


# Project01 源码包含中文注释；明确要求 MSVC 按 UTF-8 解析源文件。
if(MSVC)
    target_compile_options(oscillator_core PRIVATE /utf-8)
    target_compile_options(harmonic_oscillator PRIVATE /utf-8)
    target_compile_options(underdamped_oscillator PRIVATE /utf-8)
    target_compile_options(oscillator_batch PRIVATE /utf-8)
    target_compile_options(oscillator_benchmark PRIVATE /utf-8)
    target_compile_options(oscillator_tests PRIVATE /utf-8)
    target_compile_options(oscillator_visualizer PRIVATE /utf-8)
endif()
