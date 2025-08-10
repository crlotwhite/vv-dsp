add_library(vv-dsp-spectral spectral.c fft.c fft_kiss.c utils.c)

target_include_directories(vv-dsp-spectral PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
