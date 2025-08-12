# Google Test CMake configuration for vv-dsp

# Create test executable for window functions
add_executable(vv-dsp-gtest-window test_window.cpp)
target_link_libraries(vv-dsp-gtest-window
  PRIVATE
  vv-dsp
  gtest
  gtest_main
)

# Configure C++ standard (since we're using C++ for tests)
set_target_properties(vv-dsp-gtest-window
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# Discover and register Google Test cases with CTest
include(GoogleTest)
gtest_discover_tests(vv-dsp-gtest-window
  PROPERTIES LABELS "gtest;unit"
)
