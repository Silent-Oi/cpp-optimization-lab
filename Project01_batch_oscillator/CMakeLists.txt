add_executable(project01_oscillator
    src/main.cpp
    src/harmonic_oscillator.cpp
 "include/explicit_euler_step.h" "src/explicit_euler_step.cpp")

target_link_libraries(project01_oscillator
    PRIVATE cpp_lab_common
)

target_include_directories(project01_oscillator
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
