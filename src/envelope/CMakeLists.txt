add_library(vv-dsp-envelope
	envelope.c
	cepstrum.c
	minphase.c
	lpc.c
)

target_include_directories(vv-dsp-envelope PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)

# Envelope depends on spectral (FFT)
target_link_libraries(vv-dsp-envelope PUBLIC vv-dsp-spectral)
