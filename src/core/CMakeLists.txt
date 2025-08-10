add_library(vv-dsp-core
  core.c
)

target_include_directories(vv-dsp-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(vv-dsp-core
  PRIVATE
    $<$<BOOL:${VV_DSP_USE_SIMD}>:VV_DSP_USE_SIMD>
)
