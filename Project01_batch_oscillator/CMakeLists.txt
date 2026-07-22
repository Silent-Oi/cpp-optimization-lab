add_executable(project01_oscillator
    src/main.cpp
    src/harmonic_oscillator.cpp
    src/time_integrators.cpp
    tests/test_harmonic_oscillator.cpp
    tests/test_harmonic_oscillator.h
)

target_link_libraries(project01_oscillator
    PRIVATE cpp_lab_common
)

target_include_directories(project01_oscillator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

# Project01 源码包含中文注释；明确要求 MSVC 按 UTF-8 解析源文件。
if(MSVC)
    target_compile_options(project01_oscillator PRIVATE /utf-8)
endif()
