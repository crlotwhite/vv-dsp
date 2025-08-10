add_library(vv-dsp-resample resample.c interpolate.c resampler.c)

target_include_directories(vv-dsp-resample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
