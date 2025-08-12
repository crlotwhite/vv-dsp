add_library(vv-dsp-adapters adapters.c)

target_include_directories(vv-dsp-adapters PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)
