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

# Add tests manually to avoid GoogleTestAddTests.cmake issues on some platforms
if(MSVC)
  # For MSVC, use manual test registration to avoid path issues
  add_test(NAME WindowTest.HannWindowBasicProperties COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.HannWindowBasicProperties)
  add_test(NAME WindowTest.HammingWindowBasicProperties COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.HammingWindowBasicProperties)
  add_test(NAME WindowTest.BlackmanWindowBasicProperties COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.BlackmanWindowBasicProperties)
  add_test(NAME WindowTest.BoxcarWindowProperties COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.BoxcarWindowProperties)
  add_test(NAME WindowTest.HannWindowSpecificValues COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.HannWindowSpecificValues)
  add_test(NAME WindowTest.HammingWindowSpecificValues COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.HammingWindowSpecificValues)
  add_test(NAME WindowTest.EdgeCases COMMAND vv-dsp-gtest-window --gtest_filter=WindowTest.EdgeCases)
  add_test(NAME AllWindowTypesTest.AllWindowTypesBasicFunctionality COMMAND vv-dsp-gtest-window --gtest_filter=AllWindowTypesTest.AllWindowTypesBasicFunctionality)

  add_test(NAME FFTTest.BackendManagement COMMAND vv-dsp-gtest-fft --gtest_filter=FFTTest.BackendManagement)
  add_test(NAME FFTTest.PlanLifecycle COMMAND vv-dsp-gtest-fft --gtest_filter=FFTTest.PlanLifecycle)
  add_test(NAME FFTTest.InvalidPlanCreation COMMAND vv-dsp-gtest-fft --gtest_filter=FFTTest.InvalidPlanCreation)
  add_test(NAME FFTTest.EdgeCases COMMAND vv-dsp-gtest-fft --gtest_filter=FFTTest.EdgeCases)
  add_test(NAME FFTTest.BasicPerformanceCheck COMMAND vv-dsp-gtest-fft --gtest_filter=FFTTest.BasicPerformanceCheck)

  add_test(NAME STFTTest.STFTLifecycle COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.STFTLifecycle)
  add_test(NAME STFTTest.InvalidSTFTCreation COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.InvalidSTFTCreation)
  add_test(NAME STFTTest.SpectrogramGeneration COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.SpectrogramGeneration)
  add_test(NAME STFTTest.WindowTypeComparison COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.WindowTypeComparison)
  add_test(NAME STFTTest.EdgeCases COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.EdgeCases)
  add_test(NAME STFTTest.MinimalSizes COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.MinimalSizes)
  add_test(NAME STFTTest.PerfectReconstructionConditions COMMAND vv-dsp-gtest-stft --gtest_filter=STFTTest.PerfectReconstructionConditions)

  # Set labels for the manually added tests
  set_tests_properties(WindowTest.HannWindowBasicProperties WindowTest.HammingWindowBasicProperties
                      WindowTest.BlackmanWindowBasicProperties WindowTest.BoxcarWindowProperties
                      WindowTest.HannWindowSpecificValues WindowTest.HammingWindowSpecificValues
                      WindowTest.EdgeCases AllWindowTypesTest.AllWindowTypesBasicFunctionality
                      FFTTest.BackendManagement FFTTest.PlanLifecycle FFTTest.InvalidPlanCreation
                      FFTTest.EdgeCases FFTTest.BasicPerformanceCheck
                      STFTTest.STFTLifecycle STFTTest.InvalidSTFTCreation STFTTest.SpectrogramGeneration
                      STFTTest.WindowTypeComparison STFTTest.EdgeCases STFTTest.MinimalSizes
                      STFTTest.PerfectReconstructionConditions
                      PROPERTIES LABELS "gtest;unit")
else()
  # For non-MSVC compilers, try automatic discovery
  gtest_discover_tests(vv-dsp-gtest-window
    PROPERTIES LABELS "gtest;unit"
  )

  gtest_discover_tests(vv-dsp-gtest-fft
    PROPERTIES LABELS "gtest;unit"
  )

  gtest_discover_tests(vv-dsp-gtest-stft
    PROPERTIES LABELS "gtest;unit"
  )
endif()
