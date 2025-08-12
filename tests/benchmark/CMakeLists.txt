# Google Benchmark CMake configuration for vv-dsp

# Create benchmark executable for window functions
add_executable(vv-dsp-benchmark-window bench_window.cpp)
target_link_libraries(vv-dsp-benchmark-window
  PRIVATE
  vv-dsp
  benchmark::benchmark_main
)

# Create benchmark executable for FFT functions
add_executable(vv-dsp-benchmark-fft bench_fft.cpp)
target_link_libraries(vv-dsp-benchmark-fft
  PRIVATE
  vv-dsp
  benchmark::benchmark_main
)

# Configure C++ standard (since we're using C++ for benchmarks)
set_target_properties(vv-dsp-benchmark-window vv-dsp-benchmark-fft
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# Add benchmarks as CTest tests (they will run but not fail the build)
add_test(NAME vv-dsp-benchmark-window-run
  COMMAND $<TARGET_FILE:vv-dsp-benchmark-window> --benchmark_min_time=0.1s)

add_test(NAME vv-dsp-benchmark-fft-run
  COMMAND $<TARGET_FILE:vv-dsp-benchmark-fft> --benchmark_min_time=0.1s)

# Label benchmarks for easy identification
set_tests_properties(vv-dsp-benchmark-window-run vv-dsp-benchmark-fft-run
  PROPERTIES LABELS "benchmark;performance")

# Create a combined benchmark executable for all algorithms
add_executable(vv-dsp-benchmark-all bench_window.cpp bench_fft.cpp)
target_link_libraries(vv-dsp-benchmark-all
  PRIVATE
  vv-dsp
  benchmark::benchmark_main
)

set_target_properties(vv-dsp-benchmark-all
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

add_test(NAME vv-dsp-benchmark-all-run
  COMMAND $<TARGET_FILE:vv-dsp-benchmark-all> --benchmark_min_time=0.1s)

set_tests_properties(vv-dsp-benchmark-all-run PROPERTIES LABELS "benchmark;performance;comprehensive")
