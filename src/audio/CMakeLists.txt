add_library(vv-dsp-audio
	wav.c
)

target_include_directories(vv-dsp-audio PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)

# Link dependency on core for basic types and utilities
target_link_libraries(vv-dsp-audio PUBLIC vv-dsp-core)

# Define that audio module is enabled (for header inclusion)
target_compile_definitions(vv-dsp-audio PUBLIC VV_DSP_AUDIO_ENABLED=1)

# Optional libsndfile integration
# For now, disable libsndfile to use custom implementation
set(SNDFILE_FOUND FALSE)

if(SNDFILE_FOUND)
  message(STATUS "vv-dsp-audio: Found libsndfile, enabling libsndfile backend")
  target_compile_definitions(vv-dsp-audio PRIVATE VV_DSP_AUDIO_USE_LIBSNDFILE=1)
  target_link_libraries(vv-dsp-audio PRIVATE ${SNDFILE_LIBRARIES})
  target_include_directories(vv-dsp-audio PRIVATE ${SNDFILE_INCLUDE_DIRS})
  target_link_directories(vv-dsp-audio PRIVATE ${SNDFILE_LIBRARY_DIRS})
else()
  message(STATUS "vv-dsp-audio: libsndfile not found, using custom WAV implementation")
  target_compile_definitions(vv-dsp-audio PRIVATE VV_DSP_AUDIO_USE_LIBSNDFILE=0)
endif()
