
cmake_minimum_required(VERSION 3.15)

project(vv-dsp VERSION 0.1.0 LANGUAGES C CXX)

# C standard and compiler warnings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(VV_DSP_WERROR "Treat warnings as errors" OFF)
option(VV_DSP_UNITY_BUILD "Enable unity/jumbo builds where supported" OFF)
option(VV_DSP_IPO "Enable interprocedural optimization (LTO) for Release builds if supported" OFF)

if(MSVC)
  add_compile_options(/W4)
  # Define math constants and disable CRT security warnings for MSVC
  add_compile_definitions(_USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
else()
  # C specific compiler flags
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wconversion>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdouble-promotion>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat=2>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>)

  # C++ specific compiler flags (without -Wstrict-prototypes)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wconversion>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wdouble-promotion>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wformat=2>)

  if(VV_DSP_WERROR)
    add_compile_options(-Werror)
  endif()
endif()

# Build options
option(VV_DSP_BUILD_TESTS "Build vv-dsp tests" ON)
option(VV_DSP_BUILD_EXAMPLES "Build vv-dsp examples" ON)

# Build benchmarks only on Ubuntu/Linux
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND EXISTS "/etc/lsb-release")
  # Check if this is Ubuntu by looking for lsb-release
  file(READ "/etc/lsb-release" LSB_RELEASE_CONTENT)
  if(LSB_RELEASE_CONTENT MATCHES "DISTRIB_ID=Ubuntu")
    option(VV_DSP_BUILD_BENCHMARKS "Build vv-dsp performance benchmarks" OFF)
    set(VV_DSP_BENCHMARK_AVAILABLE TRUE)
    message(STATUS "vv-dsp: Benchmarks available on Ubuntu platform")
  else()
    set(VV_DSP_BUILD_BENCHMARKS OFF CACHE BOOL "Build vv-dsp performance benchmarks (Ubuntu only)" FORCE)
    set(VV_DSP_BENCHMARK_AVAILABLE FALSE)
    message(STATUS "vv-dsp: Benchmarks disabled (Ubuntu only)")
  endif()
else()
  set(VV_DSP_BUILD_BENCHMARKS OFF CACHE BOOL "Build vv-dsp performance benchmarks (Ubuntu only)" FORCE)
  set(VV_DSP_BENCHMARK_AVAILABLE FALSE)
  message(STATUS "vv-dsp: Benchmarks disabled (Ubuntu only)")
endif()

option(VV_DSP_USE_GTEST "Enable Google Test and Google Benchmark for modern testing" OFF)
option(VV_DSP_USE_SIMD "Enable SIMD optimizations where available" OFF)
option(VV_DSP_ENABLE_AUDIO_IO "Enable WAV audio file I/O utilities" OFF)

# FFT backend options - support multiple backends simultaneously
option(VV_DSP_WITH_KISSFFT "Enable KissFFT backend (always available)" ON)
option(VV_DSP_WITH_FFTW "Enable FFTW3 backend" OFF)
option(VV_DSP_WITH_FFTS "Enable FFTS backend" OFF)
set(VV_DSP_BACKEND_FFT "kissfft" CACHE STRING "Default FFT backend: kissfft|fftw|ffts")
set_property(CACHE VV_DSP_BACKEND_FFT PROPERTY STRINGS kissfft fftw ffts)

option(VV_DSP_SINGLE_FILE "Build as a single-file amalgamation" OFF)
option(VV_DSP_ENABLE_ASAN "Enable AddressSanitizer for sanitizing builds" OFF)
option(VV_DSP_ENABLE_UBSAN "Enable UBSanitizer for sanitizing builds" OFF)

# External dependency options
option(VV_DSP_USE_FASTAPPROX "Enable fast math approximations using fastapprox" OFF)
option(VV_DSP_USE_MATH_APPROX "Enable DSP-optimized math approximations" OFF)
option(VERIFY_WITH_PYTHON "Enable Python-based verification tests (NumPy/SciPy)" OFF)
set(VV_PY_RTOL "" CACHE STRING "Python validation relative tolerance override (e.g., 5e-5)")
set(VV_PY_ATOL "" CACHE STRING "Python validation absolute tolerance override (e.g., 5e-5)")
option(VV_PY_VERBOSE "Enable verbose output in Python validation tests" OFF)

# SIMD instruction set options - more granular control
option(VV_DSP_ENABLE_SSE4_1 "Enable SSE4.1 optimizations on x86/x64" OFF)
option(VV_DSP_ENABLE_AVX2 "Enable AVX2 optimizations on x86/x64" OFF)
option(VV_DSP_ENABLE_AVX512 "Enable AVX512 optimizations on x86/x64" OFF)
option(VV_DSP_ENABLE_NEON "Enable NEON optimizations on ARM/AArch64" OFF)

# Auto-detect SIMD capability if VV_DSP_USE_SIMD is enabled
if(VV_DSP_USE_SIMD)
  include(CheckCCompilerFlag)

  # Try to enable appropriate SIMD instruction sets based on platform
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    # Check for AVX2 support
    check_c_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2 AND NOT VV_DSP_ENABLE_AVX2)
      set(VV_DSP_ENABLE_AVX2 ON CACHE BOOL "Enable AVX2 optimizations" FORCE)
      message(STATUS "vv-dsp: Auto-enabling AVX2 optimizations")
    endif()

    # Check for SSE4.1 support
    check_c_compiler_flag("-msse4.1" COMPILER_SUPPORTS_SSE41)
    if(COMPILER_SUPPORTS_SSE41 AND NOT VV_DSP_ENABLE_SSE4_1)
      set(VV_DSP_ENABLE_SSE4_1 ON CACHE BOOL "Enable SSE4.1 optimizations" FORCE)
      message(STATUS "vv-dsp: Auto-enabling SSE4.1 optimizations")
    endif()
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    # Check for NEON support
    check_c_compiler_flag("-mfpu=neon" COMPILER_SUPPORTS_NEON)
    if(COMPILER_SUPPORTS_NEON AND NOT VV_DSP_ENABLE_NEON)
      set(VV_DSP_ENABLE_NEON ON CACHE BOOL "Enable NEON optimizations" FORCE)
      message(STATUS "vv-dsp: Auto-enabling NEON optimizations")
    endif()
  endif()
endif()

# Public compile definitions
if(VV_DSP_USE_SIMD)
  add_compile_definitions(VV_DSP_USE_SIMD=1)
endif()

# Apply SIMD compiler flags and definitions
if(VV_DSP_ENABLE_SSE4_1)
  add_compile_definitions(VV_DSP_USE_SSE4_1=1)
  if(NOT MSVC)
    add_compile_options(-msse4.1)
  endif()
  message(STATUS "vv-dsp: SSE4.1 optimizations enabled")
endif()

if(VV_DSP_ENABLE_AVX2)
  add_compile_definitions(VV_DSP_USE_AVX2=1)
  if(NOT MSVC)
    add_compile_options(-mavx2)
  else()
    add_compile_options(/arch:AVX2)
  endif()
  message(STATUS "vv-dsp: AVX2 optimizations enabled")
endif()

if(VV_DSP_ENABLE_AVX512)
  add_compile_definitions(VV_DSP_USE_AVX512=1)
  if(NOT MSVC)
    add_compile_options(-mavx512f)
  else()
    add_compile_options(/arch:AVX512)
  endif()
  message(STATUS "vv-dsp: AVX512 optimizations enabled")
endif()

if(VV_DSP_ENABLE_NEON)
  add_compile_definitions(VV_DSP_USE_NEON=1)
  if(NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    add_compile_options(-mfpu=neon)
  endif()
  message(STATUS "vv-dsp: NEON optimizations enabled")
endif()

# FFT backend compile definitions - KissFFT is always enabled
add_compile_definitions(VV_DSP_BACKEND_FFT_kissfft)

if(VV_DSP_SINGLE_FILE)
  add_compile_definitions(VV_DSP_SINGLE_FILE=1)
endif()

# External library compile definitions are added in their respective sections above

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

# IPO/LTO
include(CheckIPOSupported)
if(VV_DSP_IPO)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
  if(ipo_supported)
    message(STATUS "vv-dsp: Enabling IPO/LTO for Release configuration")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
  else()
    message(STATUS "vv-dsp: IPO not supported: ${ipo_msg}")
  endif()
endif()

# Unity builds (where supported by generator)
if(VV_DSP_UNITY_BUILD)
  set(CMAKE_UNITY_BUILD ON)
endif()

# External dependencies using FetchContent
include(FetchContent)

# FFT backend dependencies
# FFTW3 - High-performance FFT library
if(VV_DSP_WITH_FFTW)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(PC_FFTW3F QUIET fftw3f)
  endif()

  find_library(FFTW3F_LIBRARY
    NAMES fftw3f libfftw3f
    HINTS ${PC_FFTW3F_LIBDIR} ${PC_FFTW3F_LIBRARY_DIRS}
    PATH_SUFFIXES lib
  )

  find_path(FFTW3F_INCLUDE_DIR
    NAMES fftw3.h
    HINTS ${PC_FFTW3F_INCLUDEDIR} ${PC_FFTW3F_INCLUDE_DIRS}
    PATH_SUFFIXES include
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FFTW3F
    REQUIRED_VARS FFTW3F_LIBRARY FFTW3F_INCLUDE_DIR
  )

  if(FFTW3F_FOUND)
    add_library(fftw3f UNKNOWN IMPORTED)
    set_target_properties(fftw3f PROPERTIES
      IMPORTED_LOCATION "${FFTW3F_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFTW3F_INCLUDE_DIR}"
    )
    # FFTW3 requires threading support
    find_package(Threads REQUIRED)
    target_link_libraries(fftw3f INTERFACE Threads::Threads)
    message(STATUS "vv-dsp: FFTW3 backend enabled")
  else()
    message(WARNING "vv-dsp: FFTW3 requested but not found, disabling FFTW backend")
    set(VV_DSP_WITH_FFTW OFF CACHE BOOL "" FORCE)
  endif()
endif()

# FFTS - Fast FFT library
if(VV_DSP_WITH_FFTS)
  find_library(FFTS_LIBRARY
    NAMES ffts libffts
    PATH_SUFFIXES lib
  )

  find_path(FFTS_INCLUDE_DIR
    NAMES ffts.h
    PATH_SUFFIXES include
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FFTS
    REQUIRED_VARS FFTS_LIBRARY FFTS_INCLUDE_DIR
  )

  if(FFTS_FOUND)
    add_library(ffts UNKNOWN IMPORTED)
    set_target_properties(ffts PROPERTIES
      IMPORTED_LOCATION "${FFTS_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFTS_INCLUDE_DIR}"
    )
    message(STATUS "vv-dsp: FFTS backend enabled")
  else()
    message(WARNING "vv-dsp: FFTS requested but not found, disabling FFTS backend")
    set(VV_DSP_WITH_FFTS OFF CACHE BOOL "" FORCE)
  endif()
endif()

# Update compile definitions after library detection
if(VV_DSP_WITH_FFTW AND FFTW3F_FOUND)
  add_compile_definitions(VV_DSP_BACKEND_FFT_fftw)
endif()
if(VV_DSP_WITH_FFTS AND FFTS_FOUND)
  add_compile_definitions(VV_DSP_BACKEND_FFT_ffts)
endif()

# FastApprox - Fast approximations to common mathematical functions
if(VV_DSP_USE_FASTAPPROX)
  FetchContent_Declare(
    fastapprox
    GIT_REPOSITORY https://github.com/romeric/fastapprox.git
    GIT_TAG master
  )
  FetchContent_MakeAvailable(fastapprox)

  # Add fastapprox include directory
  set(FASTAPPROX_INCLUDE_DIR ${fastapprox_SOURCE_DIR}/fastapprox/src)
  add_library(fastapprox INTERFACE)
  target_include_directories(fastapprox SYSTEM INTERFACE ${FASTAPPROX_INCLUDE_DIR})

  message(STATUS "vv-dsp: FastApprox enabled")
  add_compile_definitions(VV_DSP_USE_FASTAPPROX=1)
endif()

# Math Approx - DSP-optimized math approximations
if(VV_DSP_USE_MATH_APPROX)
  FetchContent_Declare(
    math_approx
    GIT_REPOSITORY https://github.com/Chowdhury-DSP/math_approx.git
    GIT_TAG main
  )
  FetchContent_MakeAvailable(math_approx)
  message(STATUS "vv-dsp: Math Approx (DSP-optimized) enabled")
  add_compile_definitions(VV_DSP_USE_MATH_APPROX=1)
endif()

# Google Test and Google Benchmark (when VV_DSP_USE_GTEST is enabled)
if(VV_DSP_USE_GTEST)
  # Store original flags
  set(ORIG_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(ORIG_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

  # Disable problematic flags for external dependencies
  string(REGEX REPLACE "-Werror[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "-Wconversion[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "-Wdouble-promotion[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "-Wformat=[0-9][^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  # Set GoogleTest options to build static libs and avoid installing
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Set Google Benchmark options
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

  # Fetch GoogleTest
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )

  # Fetch Google Benchmark
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
  )

  FetchContent_MakeAvailable(googletest googlebenchmark)

  # Restore original flags for our own code
  set(CMAKE_CXX_FLAGS "${ORIG_CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${ORIG_CMAKE_C_FLAGS}")

  message(STATUS "vv-dsp: Google Test and Google Benchmark enabled")
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
add_subdirectory(src/features)
if(VV_DSP_ENABLE_AUDIO_IO)
  add_subdirectory(src/audio)
endif()
add_subdirectory(tools)

# Benchmarks
if(VV_DSP_BUILD_BENCHMARKS AND VV_DSP_BENCHMARK_AVAILABLE)
  add_subdirectory(bench)
elseif(VV_DSP_BUILD_BENCHMARKS)
  message(WARNING "vv-dsp: Benchmarks are only supported on Ubuntu platforms")
endif()

# Umbrella interface target that aggregates module libraries
add_library(vv-dsp INTERFACE)
target_include_directories(vv-dsp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Link external dependencies to umbrella target
if(VV_DSP_USE_FASTAPPROX AND TARGET fastapprox)
  target_link_libraries(vv-dsp INTERFACE fastapprox)
endif()
if(VV_DSP_USE_MATH_APPROX AND TARGET math_approx)
  target_link_libraries(vv-dsp INTERFACE math_approx)
endif()

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
if(TARGET vv-dsp-features)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-features)
endif()
if(TARGET vv-dsp-audio)
  target_link_libraries(vv-dsp INTERFACE vv-dsp-audio)
endif()

# Testing
if(VV_DSP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)

  # Add Google Test and Google Benchmark subdirectories if enabled
  if(VV_DSP_USE_GTEST)
    add_subdirectory(tests/gtest)
    add_subdirectory(tests/benchmark)
  endif()

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

  add_executable(vv-dsp-fastapprox-example examples/fastapprox_example.c)
  target_link_libraries(vv-dsp-fastapprox-example PRIVATE vv-dsp)
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
  set(CZT_BIN $<TARGET_FILE:vv_dsp_dump_czt>)
  set(DCT_BIN $<TARGET_FILE:vv_dsp_dump_dct>)
  set(STATS_BIN $<TARGET_FILE:vv_dsp_dump_stats>)

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
      add_test(NAME py-czt
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_czt.py
          --czt-bin ${CZT_BIN})
      add_test(NAME py-dct
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_dct.py
          --dct-bin ${DCT_BIN})
      add_test(NAME py-stats
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/python/test_stats.py
          --stats-bin ${STATS_BIN})

      # Label tests for easy selection
  set_tests_properties(py-fft PROPERTIES LABELS "py;validation")
  set_tests_properties(py-filters PROPERTIES LABELS "py;validation")
  set_tests_properties(py-stft PROPERTIES LABELS "py;validation")
  set_tests_properties(py-resampler PROPERTIES LABELS "py;validation")
  set_tests_properties(py-dct PROPERTIES LABELS "py;validation")
  set_tests_properties(py-stats PROPERTIES LABELS "py;validation")
  set_tests_properties(py-czt PROPERTIES LABELS "py;validation")

      # Treat Python scripts' exit code 77 as SKIP in CTest
  set_tests_properties(py-fft py-filters py-stft py-resampler py-dct py-czt py-stats
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
          py-fft py-filters py-stft py-resampler py-dct py-czt py-stats
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

# Package configuration for installation
include(CMakePackageConfigHelpers)

# Set CMake install paths
include(GNUInstallDirs)

# Collect all targets for export
set(VV_DSP_TARGETS vv-dsp)

# Add module targets if they exist
if(TARGET vv-dsp-core)
  list(APPEND VV_DSP_TARGETS vv-dsp-core)
endif()
if(TARGET vv-dsp-spectral)
  list(APPEND VV_DSP_TARGETS vv-dsp-spectral)
endif()
if(TARGET vv-dsp-filter)
  list(APPEND VV_DSP_TARGETS vv-dsp-filter)
endif()
if(TARGET vv-dsp-resample)
  list(APPEND VV_DSP_TARGETS vv-dsp-resample)
endif()
if(TARGET vv-dsp-envelope)
  list(APPEND VV_DSP_TARGETS vv-dsp-envelope)
endif()
if(TARGET vv-dsp-window)
  list(APPEND VV_DSP_TARGETS vv-dsp-window)
endif()
if(TARGET vv-dsp-adapters)
  list(APPEND VV_DSP_TARGETS vv-dsp-adapters)
endif()
if(TARGET vv-dsp-features)
  list(APPEND VV_DSP_TARGETS vv-dsp-features)
endif()
if(TARGET vv-dsp-audio)
  list(APPEND VV_DSP_TARGETS vv-dsp-audio)
endif()

# Install all targets
install(TARGETS ${VV_DSP_TARGETS}
  EXPORT vv-dsp-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install header files
install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

# Install CMake targets file
install(EXPORT vv-dsp-targets
  FILE vv-dsp-targets.cmake
  NAMESPACE vv-dsp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vv-dsp
)

# Generate and install config files
configure_package_config_file(
  cmake/vv-dsp-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/vv-dsp-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vv-dsp
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/vv-dsp-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/vv-dsp-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/vv-dsp-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vv-dsp
)
