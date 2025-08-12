# Benchmark suite for vv-dsp
cmake_minimum_required(VERSION 3.15)

# Check if building benchmarks is enabled
if(NOT VV_DSP_BUILD_BENCHMARKS)
    return()
endif()

# Benchmark executable
add_executable(vv_dsp_bench
    vv_dsp_bench.c
    bench_timer.c
    bench_framework.c
    bench_stft.c
    bench_filter_fixed.c
    bench_resample_fixed.c
    bench_pipeline.c
    bench_denormals.c
)

# Additional benchmark utilities
add_executable(benchmark_simd benchmark_simd.c)
target_link_libraries(benchmark_simd PRIVATE vv-dsp)

add_executable(simple_bench
    simple_bench.c
    bench_framework.c
    bench_stft.c
    bench_timer.c
)
target_link_libraries(simple_bench PRIVATE vv-dsp)

# Vectorized math benchmark
add_executable(bench_vectorized_math bench_vectorized_math.c)
target_link_libraries(bench_vectorized_math PRIVATE vv-dsp)

# Accuracy-performance trade-offs benchmark
add_executable(bench_accuracy_performance_trade_offs bench_accuracy_performance_trade_offs.c)
target_link_libraries(bench_accuracy_performance_trade_offs PRIVATE vv-dsp)

# Link against the vv-dsp umbrella target
# TEMPORARY: Link directly to avoid Eigen/C++ issues
target_link_libraries(vv_dsp_bench PRIVATE
    vv-dsp-core
    vv-dsp-spectral
    vv-dsp-window
    vv-dsp-filter
    vv-dsp-resample
    vv-dsp-envelope
    vv-dsp-features
    vv-dsp-adapters
)

# Include directories
target_include_directories(vv_dsp_bench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VV_DSP_INCLUDE_DIR}
)

# Apply the same include directories to additional benchmarks
target_include_directories(benchmark_simd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VV_DSP_INCLUDE_DIR}
)

target_include_directories(simple_bench PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VV_DSP_INCLUDE_DIR}
)

target_include_directories(bench_vectorized_math PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VV_DSP_INCLUDE_DIR}
)

target_include_directories(bench_accuracy_performance_trade_offs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VV_DSP_INCLUDE_DIR}
)

# Platform-specific linking
if(WIN32)
    # Windows: Link with winmm for high-resolution timing
    target_link_libraries(vv_dsp_bench PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
    # Linux: Link with real-time library for clock_gettime
    target_link_libraries(vv_dsp_bench PRIVATE rt)
elseif(APPLE)
    # macOS: No additional libraries needed
endif()

# Copy benchmark to build directory for easy access
set_target_properties(vv_dsp_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Set the same properties for additional benchmarks
set_target_properties(benchmark_simd PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(simple_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(bench_vectorized_math PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(bench_accuracy_performance_trade_offs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add benchmark target to 'bench' group
set_target_properties(vv_dsp_bench PROPERTIES
    FOLDER "Benchmarks"
)

# Optional: Create a custom target to run benchmarks
add_custom_target(run_benchmarks
    COMMAND vv_dsp_bench --format=json --output=benchmark_results.json
    DEPENDS vv_dsp_bench
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running vv-dsp benchmark suite"
)
