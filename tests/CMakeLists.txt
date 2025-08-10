add_executable(vv-dsp-sanity-test sanity.c)

target_link_libraries(vv-dsp-sanity-test PRIVATE
  vv-dsp
)

add_test(NAME vv-dsp-sanity COMMAND vv-dsp-sanity-test)

# Core tests
add_executable(vv-dsp-core-tests core_tests.c)
target_link_libraries(vv-dsp-core-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-core COMMAND vv-dsp-core-tests)
