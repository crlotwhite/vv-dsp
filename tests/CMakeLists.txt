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

# FP Environment tests
add_executable(vv-dsp-fp-env-tests test_fp_env.c)
target_link_libraries(vv-dsp-fp-env-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-fp-env COMMAND $<TARGET_FILE:vv-dsp-fp-env-tests>)

# Framing tests
add_executable(vv-dsp-framing-tests framing_tests.c)
target_link_libraries(vv-dsp-framing-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-framing COMMAND $<TARGET_FILE:vv-dsp-framing-tests>)

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

# Simple micro-benchmark (C implementation only)
add_executable(vv-dsp-bench bench_fft_simple.c)
target_link_libraries(vv-dsp-bench PRIVATE vv-dsp)
# You can run manually: ctest -R bench -V or just execute vv-dsp-bench binary
add_test(NAME vv-dsp-bench-run COMMAND $<TARGET_FILE:vv-dsp-bench>)
set_tests_properties(vv-dsp-bench-run PROPERTIES DISABLED TRUE)

# CZT tests
add_executable(vv-dsp-czt-tests czt_tests.c)
target_link_libraries(vv-dsp-czt-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-czt COMMAND $<TARGET_FILE:vv-dsp-czt-tests>)

# FFT Backend tests
add_executable(vv-dsp-fft-backend-tests fft_backend_tests.c)
target_link_libraries(vv-dsp-fft-backend-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-fft-backend COMMAND $<TARGET_FILE:vv-dsp-fft-backend-tests>)

# Hilbert tests
add_executable(vv-dsp-hilbert-tests hilbert_tests.c)
target_link_libraries(vv-dsp-hilbert-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-hilbert COMMAND $<TARGET_FILE:vv-dsp-hilbert-tests>)

# MFCC/Features tests
add_executable(vv-dsp-mfcc-tests mfcc_tests.c)
target_link_libraries(vv-dsp-mfcc-tests PRIVATE vv-dsp)
add_test(NAME vv-dsp-mfcc COMMAND $<TARGET_FILE:vv-dsp-mfcc-tests>)

# SIMD tests (if SIMD support is enabled)
if(VV_DSP_USE_SIMD)
  add_executable(vv-dsp-simd-core-tests test_simd_core.c)
  target_link_libraries(vv-dsp-simd-core-tests PRIVATE vv-dsp)
  add_test(NAME vv-dsp-simd-core COMMAND $<TARGET_FILE:vv-dsp-simd-core-tests>)

  add_executable(vv-dsp-simd-memory-tests test_simd_memory.c)
  target_link_libraries(vv-dsp-simd-memory-tests PRIVATE vv-dsp)
  add_test(NAME vv-dsp-simd-memory COMMAND $<TARGET_FILE:vv-dsp-simd-memory-tests>)
endif()

# Additional debug/test utilities
add_executable(vv-dsp-debug-stft debug_stft.c)
target_link_libraries(vv-dsp-debug-stft PRIVATE vv-dsp)

add_executable(vv-dsp-test-stft-simple test_stft_simple.c)
target_link_libraries(vv-dsp-test-stft-simple PRIVATE vv-dsp)
