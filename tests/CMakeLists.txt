add_executable(vv-dsp-sanity-test sanity.c)

target_link_libraries(vv-dsp-sanity-test PRIVATE
  vv-dsp
)

# Use generator expression to ensure proper path on multi-config generators (e.g., MSVC)
add_test(NAME vv-dsp-sanity COMMAND $<TARGET_FILE:vv-dsp-sanity-test>)

# Core tests
add_executable(vv-dsp-core-tests core_tests.c)
target_link_libraries(vv-dsp-core-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-core COMMAND $<TARGET_FILE:vv-dsp-core-tests>)

# Spectral tests
add_executable(vv-dsp-spectral-tests spectral_tests.c)
target_link_libraries(vv-dsp-spectral-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-spectral COMMAND $<TARGET_FILE:vv-dsp-spectral-tests>)
# DCT tests
add_executable(vv-dsp-dct-tests dct_tests.c)
target_link_libraries(vv-dsp-dct-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-dct COMMAND $<TARGET_FILE:vv-dsp-dct-tests>)

# Window tests
add_executable(vv-dsp-window-tests window_tests.c)
target_link_libraries(vv-dsp-window-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-window COMMAND $<TARGET_FILE:vv-dsp-window-tests>)

# Resample tests
add_executable(vv-dsp-resample-tests resample_tests.c)
target_link_libraries(vv-dsp-resample-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-resample COMMAND $<TARGET_FILE:vv-dsp-resample-tests>)

# Filter tests
add_executable(vv-dsp-filter-tests filter_tests.c)
target_link_libraries(vv-dsp-filter-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-filter COMMAND $<TARGET_FILE:vv-dsp-filter-tests>)

# Savitzky–Golay tests
add_executable(vv-dsp-savgol-tests savgol_tests.c)
target_link_libraries(vv-dsp-savgol-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-savgol COMMAND $<TARGET_FILE:vv-dsp-savgol-tests>)

add_executable(vv-dsp-savgol-basic-tests savgol_basic_func_tests.c)
target_link_libraries(vv-dsp-savgol-basic-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-savgol-basic COMMAND $<TARGET_FILE:vv-dsp-savgol-basic-tests>)

# Envelope tests
add_executable(vv-dsp-envelope-tests envelope_tests.c)
target_link_libraries(vv-dsp-envelope-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-envelope COMMAND $<TARGET_FILE:vv-dsp-envelope-tests>)

# Audio tests (only if audio module is enabled)
if(TARGET vv-dsp-audio)
  add_executable(vv-dsp-audio-tests audio_tests.c)
  target_link_libraries(vv-dsp-audio-tests PRIVATE vv-dsp)
  add_test(NAME vv-dsp-audio COMMAND $<TARGET_FILE:vv-dsp-audio-tests>)
endif()

# C++ sanity compile test for RAII wrapper skeleton
add_executable(vv-dsp-cpp-sanity tests_cpp_sanity.cpp)
target_link_libraries(vv-dsp-cpp-sanity PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-sanity COMMAND $<TARGET_FILE:vv-dsp-cpp-sanity>)

# C++ wrapper functional smoke tests
add_executable(vv-dsp-cpp-fft test_cpp_fft.cpp)
target_link_libraries(vv-dsp-cpp-fft PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-fft COMMAND $<TARGET_FILE:vv-dsp-cpp-fft>)

add_executable(vv-dsp-cpp-misc test_cpp_misc.cpp)
target_link_libraries(vv-dsp-cpp-misc PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-misc COMMAND $<TARGET_FILE:vv-dsp-cpp-misc>)

add_executable(vv-dsp-cpp-math test_cpp_math.cpp)
target_link_libraries(vv-dsp-cpp-math PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-math COMMAND $<TARGET_FILE:vv-dsp-cpp-math>)

add_executable(vv-dsp-cpp-iir test_cpp_iir.cpp)
target_link_libraries(vv-dsp-cpp-iir PRIVATE vv-dsp)
add_test(NAME vv-dsp-cpp-iir COMMAND $<TARGET_FILE:vv-dsp-cpp-iir>)

# Simple micro-benchmark (not part of pass/fail by default)
add_executable(vv-dsp-bench bench_fft_stft.cpp)
target_link_libraries(vv-dsp-bench PRIVATE vv-dsp)
# You can run manually: ctest -R bench -V or just execute vv-dsp-bench binary
add_test(NAME vv-dsp-bench-run COMMAND $<TARGET_FILE:vv-dsp-bench>)
set_tests_properties(vv-dsp-bench-run PROPERTIES DISABLED TRUE)

# CZT tests
add_executable(vv-dsp-czt-tests czt_tests.c)
target_link_libraries(vv-dsp-czt-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-czt COMMAND $<TARGET_FILE:vv-dsp-czt-tests>)

# Hilbert tests
add_executable(vv-dsp-hilbert-tests hilbert_tests.c)
target_link_libraries(vv-dsp-hilbert-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-hilbert COMMAND $<TARGET_FILE:vv-dsp-hilbert-tests>)

# MFCC/Features tests
add_executable(vv-dsp-mfcc-tests mfcc_tests.c)
target_link_libraries(vv-dsp-mfcc-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-mfcc COMMAND $<TARGET_FILE:vv-dsp-mfcc-tests>)
