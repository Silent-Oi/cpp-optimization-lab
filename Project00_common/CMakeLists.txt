add_library(cpp_lab_common STATIC
    tests/test_common.cpp
 "tests/test_common.h")

target_include_directories(
    cpp_lab_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

if(MSVC)
    target_compile_options(cpp_lab_common PRIVATE /utf-8)
endif()
