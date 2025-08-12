add_library(vv-dsp-resample resample.c interpolate.c resampler.c)

target_include_directories(vv-dsp-resample PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)
