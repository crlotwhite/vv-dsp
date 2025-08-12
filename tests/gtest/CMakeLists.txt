# Google Test CMake configuration for vv-dsp

# Create test executable for window functions
add_executable(vv-dsp-gtest-window test_window.cpp)
target_link_libraries(vv-dsp-gtest-window
  PRIVATE
  vv-dsp
  gtest
  gtest_main
)

# Create test executable for FFT functions
add_executable(vv-dsp-gtest-fft test_fft.cpp)
target_link_libraries(vv-dsp-gtest-fft
  PRIVATE
  vv-dsp
  gtest
  gtest_main
)

# Create test executable for STFT functions
add_executable(vv-dsp-gtest-stft test_stft.cpp)
target_link_libraries(vv-dsp-gtest-stft
  PRIVATE
  vv-dsp
  gtest
  gtest_main
)

# Configure C++ standard (since we're using C++ for tests)
set_target_properties(vv-dsp-gtest-window vv-dsp-gtest-fft vv-dsp-gtest-stft
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# Discover and register Google Test cases with CTest
include(GoogleTest)
gtest_discover_tests(vv-dsp-gtest-window
  PROPERTIES LABELS "gtest;unit"
)

gtest_discover_tests(vv-dsp-gtest-fft
  PROPERTIES LABELS "gtest;unit"
)

gtest_discover_tests(vv-dsp-gtest-stft
  PROPERTIES LABELS "gtest;unit"
)
