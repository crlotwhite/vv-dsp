add_library(vv-dsp-core
  core.c
  stats.c
  framing.c
  nan_policy.c
  fp_env.c
  simd_memory.c
  simd_core.c
)

# Add vectorized math module if Eigen is available
if(VV_DSP_USE_EIGEN)
  target_sources(vv-dsp-core PRIVATE vv_dsp_vectorized_math.cpp)
  # Ensure C++ compilation for this source
  set_source_files_properties(vv_dsp_vectorized_math.cpp PROPERTIES LANGUAGE CXX)
endif()

target_include_directories(vv-dsp-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(vv-dsp-core
  PRIVATE
    $<$<BOOL:${VV_DSP_USE_SIMD}>:VV_DSP_USE_SIMD>
    $<$<BOOL:${VV_DSP_USE_EIGEN}>:VV_DSP_USE_EIGEN>
)

# Link math library on Unix-like systems (required on Linux/Ubuntu CI)
if(UNIX)
  target_link_libraries(vv-dsp-core PUBLIC m)
endif()

# Link Eigen if available
if(VV_DSP_USE_EIGEN AND TARGET Eigen3::Eigen)
  target_link_libraries(vv-dsp-core PUBLIC Eigen3::Eigen)
endif()
