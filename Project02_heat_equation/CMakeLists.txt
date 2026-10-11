add_library(heat_core STATIC
    src/heat_diffusion.cpp
)

target_include_directories(heat_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(heat_core PUBLIC cpp_lab_common)

if(MSVC)
  target_compile_options(heat_core PRIVATE /Qvec-report:2 /FAsu)
    target_compile_options(heat_core PRIVATE /utf-8)
endif()

# Add executable targets with the CPU implementation.
# CUDA is introduced in stage 4; configuring the CPU workspace needs no toolkit.
