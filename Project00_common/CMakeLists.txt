# Project00 currently exposes only the common include directory.
# Source files and compiled targets can be added when they are actually needed.

add_library(cpp_lab_common INTERFACE)

target_include_directories(
    cpp_lab_common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(cpp_lab_common INTERFACE cxx_std_20)
