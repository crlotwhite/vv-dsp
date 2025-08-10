cmake_minimum_required(VERSION 3.15)

project(vv-dsp VERSION 0.1.0 LANGUAGES C)

# C standard and compiler warnings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build options
option(VV_DSP_BUILD_TESTS "Build vv-dsp tests" ON)
option(VV_DSP_BUILD_EXAMPLES "Build vv-dsp examples" ON)
option(VV_DSP_USE_SIMD "Enable SIMD optimizations where available" OFF)
set(VV_DSP_BACKEND_FFT "kissfft" CACHE STRING "FFT backend to use: none|fftw|kissfft")
set_property(CACHE VV_DSP_BACKEND_FFT PROPERTY STRINGS none fftw kissfft)
option(VV_DSP_SINGLE_FILE "Build as a single-file amalgamation" OFF)

# Public compile definitions
if(VV_DSP_USE_SIMD)
  add_compile_definitions(VV_DSP_USE_SIMD=1)
endif()

add_compile_definitions(
  VV_DSP_BACKEND_FFT_${VV_DSP_BACKEND_FFT}
)

if(VV_DSP_SINGLE_FILE)
  add_compile_definitions(VV_DSP_SINGLE_FILE=1)
endif()

# Include directories
set(VV_DSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Subdirectories for modules
add_subdirectory(src/core)
add_subdirectory(src/spectral)
add_subdirectory(src/filter)
add_subdirectory(src/resample)
add_subdirectory(src/envelope)
add_subdirectory(src/window)
add_subdirectory(src/adapters)

# Umbrella interface target that aggregates module libraries
add_library(vv-dsp INTERFACE)
target_include_directories(vv-dsp INTERFACE ${VV_DSP_INCLUDE_DIR})
# Link interface to module libs so consumers only link vv-dsp
if(TARGET vv-dsp-core)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-core)
endif()
if(TARGET vv-dsp-spectral)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-spectral)
endif()
if(TARGET vv-dsp-filter)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-filter)
endif()
if(TARGET vv-dsp-resample)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-resample)
endif()
if(TARGET vv-dsp-envelope)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-envelope)
endif()
if(TARGET vv-dsp-window)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-window)
endif()
if(TARGET vv-dsp-adapters)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-adapters)
endif()

# Testing
if(VV_DSP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Examples
if(VV_DSP_BUILD_EXAMPLES)
  add_executable(vv-dsp-filter-example examples/filter_example.c)
  target_link_libraries(vv-dsp-filter-example PRIVATE vv-dsp)
endif()
