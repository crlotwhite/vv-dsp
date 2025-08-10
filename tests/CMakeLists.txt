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
