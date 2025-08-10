add_executable(vv-dsp-sanity-test sanity.c)

target_link_libraries(vv-dsp-sanity-test PRIVATE
  vv-dsp
)

add_test(NAME vv-dsp-sanity COMMAND vv-dsp-sanity-test)

# Core tests
add_executable(vv-dsp-core-tests core_tests.c)
target_link_libraries(vv-dsp-core-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-core COMMAND vv-dsp-core-tests)

# Spectral tests
add_executable(vv-dsp-spectral-tests spectral_tests.c)
target_link_libraries(vv-dsp-spectral-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-spectral COMMAND vv-dsp-spectral-tests)

# Window tests
add_executable(vv-dsp-window-tests window_tests.c)
target_link_libraries(vv-dsp-window-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-window COMMAND vv-dsp-window-tests)

# Resample tests
add_executable(vv-dsp-resample-tests resample_tests.c)
target_link_libraries(vv-dsp-resample-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-resample COMMAND vv-dsp-resample-tests)

# Filter tests
add_executable(vv-dsp-filter-tests filter_tests.c)
target_link_libraries(vv-dsp-filter-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-filter COMMAND vv-dsp-filter-tests)

# Envelope tests
add_executable(vv-dsp-envelope-tests envelope_tests.c)
target_link_libraries(vv-dsp-envelope-tests PRIVATE vv-dsp)
if(UNIX)
  target_link_libraries(vv-dsp-envelope-tests PRIVATE m)
endif()
add_test(NAME vv-dsp-envelope COMMAND vv-dsp-envelope-tests)

# C++ sanity compile test for RAII wrapper skeleton
add_executable(vv-dsp-cpp-sanity tests_cpp_sanity.cpp)
target_link_libraries(vv-dsp-cpp-sanity PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-sanity COMMAND vv-dsp-cpp-sanity)

# C++ wrapper functional smoke tests
add_executable(vv-dsp-cpp-fft test_cpp_fft.cpp)
target_link_libraries(vv-dsp-cpp-fft PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-fft COMMAND vv-dsp-cpp-fft)

add_executable(vv-dsp-cpp-misc test_cpp_misc.cpp)
target_link_libraries(vv-dsp-cpp-misc PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-misc COMMAND vv-dsp-cpp-misc)

add_executable(vv-dsp-cpp-math test_cpp_math.cpp)
target_link_libraries(vv-dsp-cpp-math PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-math COMMAND vv-dsp-cpp-math)

add_executable(vv-dsp-cpp-iir test_cpp_iir.cpp)
target_link_libraries(vv-dsp-cpp-iir PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-iir COMMAND vv-dsp-cpp-iir)
