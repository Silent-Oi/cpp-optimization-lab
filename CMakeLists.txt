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

option(CPP_LAB_BUILD_PROJECT01 "Build Project01" ON)
option(CPP_LAB_BUILD_PROJECT02 "Build Project02" ON)
add_subdirectory(Project00_common)

if(CPP_LAB_BUILD_PROJECT01)
    add_subdirectory(Project01_batch_oscillator)
endif()

if(CPP_LAB_BUILD_PROJECT02)
    if(MSVC)
        string(APPEND CMAKE_CUDA_FLAGS_INIT " -Xcompiler=/utf-8,/Zc:preprocessor")
    endif()
    enable_language(CUDA)
    add_subdirectory(Project02_heat_equation)
endif()
