add_library(heat_core STATIC
    src/heat_diffusion.cpp
    cuda/cuda_heat_diffusion.cu
    cuda/cuda_heat_diffusion.h
)

target_include_directories(heat_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cuda
    PRIVATE
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)
target_link_libraries(heat_core PUBLIC cpp_lab_common)

target_compile_definitions(
    heat_core
    PUBLIC
        PROJECT02_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(heat_core PROPERTIES
    CUDA_STANDARD 20
    CUDA_STANDARD_REQUIRED ON
    CUDA_EXTENSIONS OFF
)

target_compile_options(heat_core PRIVATE
    "$<$<COMPILE_LANGUAGE:CUDA>:--utf-8>"
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

add_executable(heat_visualizer
    "visualizer/visualizer_main.cpp"
    visualizer/pixel_buffer.cpp
    visualizer/heatmap_renderer.h
    visualizer/heatmap_renderer.cpp
    visualizer/rgfw_impl.cpp
)

target_include_directories(heat_visualizer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/visualizer
        ${CMAKE_SOURCE_DIR}/third_party/RGFW
)

target_link_libraries(heat_visualizer
    PRIVATE heat_core
)


add_executable(cuda_basics
    cuda/cuda_basics.cu
)

set_target_properties(cuda_basics PROPERTIES
    CUDA_STANDARD 20
    CUDA_STANDARD_REQUIRED ON
    CUDA_EXTENSIONS OFF
)
target_include_directories(cuda_basics PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})


if(MSVC)
    target_compile_options(heat_core PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:/Qvec-report:2;/FAsu;/utf-8>"
    )
    target_compile_options(heat_benchmark PRIVATE /utf-8)
    target_compile_options(heat_tests PRIVATE /utf-8)
    target_compile_options(heat_visualizer PRIVATE /utf-8)
endif()
