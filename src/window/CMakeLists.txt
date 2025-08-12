add_library(vv-dsp-window window.c)

target_include_directories(vv-dsp-window PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)
