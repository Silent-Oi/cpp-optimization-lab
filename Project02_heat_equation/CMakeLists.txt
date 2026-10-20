add_library(heat_core STATIC
    src/heat_diffusion.cpp
)

target_include_directories(heat_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(heat_core PUBLIC cpp_lab_common)

target_compile_definitions(
    heat_core
    PUBLIC
        PROJECT02_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)


add_executable(heat_benchmark
    benchmarks/heat_benchmark.cpp
)

target_link_libraries(heat_benchmark
    PRIVATE heat_core
)


add_executable(heat_tests
    tests/test_main.cpp
    tests/test_heat_diffusion.cpp
)

target_include_directories(heat_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(heat_tests
    PRIVATE heat_core
)

if(MSVC)
    target_compile_options(heat_core PRIVATE /Qvec-report:2 /FAsu)
    target_compile_options(heat_core PRIVATE /utf-8)
    target_compile_options(heat_benchmark PRIVATE /utf-8)
    target_compile_options(heat_tests PRIVATE /utf-8)
endif()

# Add executable targets with the CPU implementation.
# CUDA is introduced in stage 4; configuring the CPU workspace needs no toolkit.
