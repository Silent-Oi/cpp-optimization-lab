# Expose headers until the first CPU implementation is added.
add_library(heat_core INTERFACE)

target_include_directories(heat_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(heat_core INTERFACE cpp_lab_common)

if(MSVC)
    target_compile_options(heat_core INTERFACE /utf-8)
endif()

# Add compiled sources and executable targets with the CPU implementation.
# CUDA is introduced in stage 4; configuring the CPU workspace needs no toolkit.
