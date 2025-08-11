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
)

# Link against the vv-dsp umbrella target
target_link_libraries(vv_dsp_bench PRIVATE vv-dsp)

# Include directories
target_include_directories(vv_dsp_bench PRIVATE
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
