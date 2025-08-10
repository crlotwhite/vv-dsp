cmake_minimum_required(VERSION 3.15)

project(vv-dsp VERSION 0.1.0 LANGUAGES C CXX)

# C standard and compiler warnings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
option(VV_DSP_ENABLE_ASAN "Enable AddressSanitizer for sanitizing builds" OFF)
option(VV_DSP_ENABLE_UBSAN "Enable UBSanitizer for sanitizing builds" OFF)
option(VERIFY_WITH_PYTHON "Enable Python-based verification tests (NumPy/SciPy)" OFF)
set(VV_PY_RTOL "" CACHE STRING "Python validation relative tolerance override (e.g., 5e-5)")
set(VV_PY_ATOL "" CACHE STRING "Python validation absolute tolerance override (e.g., 5e-5)")
option(VV_PY_VERBOSE "Enable verbose output in Python validation tests" OFF)

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

# Sanitizers (non-MSVC)
if(NOT MSVC)
  if(VV_DSP_ENABLE_ASAN)
    message(STATUS "vv-dsp: AddressSanitizer enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
  if(VV_DSP_ENABLE_UBSAN)
    message(STATUS "vv-dsp: UBSanitizer enabled")
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  endif()
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
add_subdirectory(tools)

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
  # Optional: configure CTest MemCheck to use valgrind if available
  find_program(VALGRIND_EXECUTABLE valgrind)
  if(VALGRIND_EXECUTABLE)
    set(CTEST_MEMORYCHECK_COMMAND ${VALGRIND_EXECUTABLE})
    set(MEMORYCHECK_COMMAND ${VALGRIND_EXECUTABLE})
    set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tests/valgrind.supp" CACHE FILEPATH "Valgrind suppressions file")
  endif()
endif()

# Examples
if(VV_DSP_BUILD_EXAMPLES)
  add_executable(vv-dsp-filter-example examples/filter_example.c)
  target_link_libraries(vv-dsp-filter-example PRIVATE vv-dsp)
endif()

# Python-based verification harness (optional, opt-in via VERIFY_WITH_PYTHON)
if(VV_DSP_BUILD_TESTS AND VERIFY_WITH_PYTHON)
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    # Check NumPy/SciPy availability by attempting import
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import numpy, scipy"
      RESULT_VARIABLE _py_import_rc
      OUTPUT_QUIET ERROR_QUIET
    )
    if(_py_import_rc EQUAL 0)
      # Pass fully-qualified binary paths to Python scripts
      set(FFT_BIN $<TARGET_FILE:vv_dsp_dump_fft>)
      set(FIR_BIN $<TARGET_FILE:vv_dsp_dump_fir>)
      set(IIR_BIN $<TARGET_FILE:vv_dsp_dump_iir>)
      set(STFT_BIN $<TARGET_FILE:vv_dsp_dump_stft_roundtrip>)
      set(RESAMPLE_BIN $<TARGET_FILE:vv_dsp_dump_resample>)

      add_test(NAME py-fft
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_fft.py
          --fft-bin ${FFT_BIN})
      add_test(NAME py-filters
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_filters.py
          --fir-bin ${FIR_BIN} --iir-bin ${IIR_BIN})
      add_test(NAME py-stft
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_stft.py
          --stft-bin ${STFT_BIN})
      add_test(NAME py-resampler
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_resampler.py
          --resample-bin ${RESAMPLE_BIN})

      # Label tests for easy selection
      set_tests_properties(py-fft PROPERTIES LABELS "py;validation")
      set_tests_properties(py-filters PROPERTIES LABELS "py;validation")
      set_tests_properties(py-stft PROPERTIES LABELS "py;validation")
      set_tests_properties(py-resampler PROPERTIES LABELS "py;validation")

      # Treat Python scripts' exit code 77 as SKIP in CTest
      set_tests_properties(py-fft py-filters py-stft py-resampler
        PROPERTIES SKIP_RETURN_CODE 77)

      # Make Python tests friendlier with sanitizers when enabled
      set(_vv_py_env "")
      if(VV_DSP_ENABLE_ASAN)
        # Reduce false positives from Python allocation model
        list(APPEND _vv_py_env "ASAN_OPTIONS=detect_leaks=0:disable_coredump=1")
      endif()
      if(VV_DSP_ENABLE_UBSAN)
        list(APPEND _vv_py_env "UBSAN_OPTIONS=print_stacktrace=1")
      endif()
      # Expose tolerance/verbosity controls to Python via env
      if(VV_PY_VERBOSE)
        list(APPEND _vv_py_env "VV_PY_VERBOSE=1")
      endif()
      if(NOT VV_PY_RTOL STREQUAL "")
        list(APPEND _vv_py_env "VV_PY_RTOL=${VV_PY_RTOL}")
      endif()
      if(NOT VV_PY_ATOL STREQUAL "")
        list(APPEND _vv_py_env "VV_PY_ATOL=${VV_PY_ATOL}")
      endif()
      if(_vv_py_env)
        string(JOIN ";" _vv_py_env_str ${_vv_py_env})
        set_tests_properties(
          py-fft py-filters py-stft py-resampler
          PROPERTIES ENVIRONMENT "${_vv_py_env_str}"
        )
      endif()
    else()
      message(STATUS "Python3 found, but NumPy/SciPy not importable; skipping Python validation tests. Enable by installing requirements.txt or set VERIFY_WITH_PYTHON=OFF to silence.")
    endif()
  else()
    message(STATUS "Python3 interpreter not found; Python-based verification tests will be skipped by CTest.")
  endif()
endif()
