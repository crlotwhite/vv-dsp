set(VV_DSP_SPECTRAL_SOURCES
  spectral.c
  fft.c
  fft_kiss.c
  utils.c
  stft.c
  dct.c
  czt.c
  hilbert.c
)

# Add FFT backend sources conditionally
if(VV_DSP_WITH_FFTW AND FFTW3F_FOUND)
  list(APPEND VV_DSP_SPECTRAL_SOURCES fft_fftw.c)
endif()

if(VV_DSP_WITH_FFTS AND FFTS_FOUND)
  list(APPEND VV_DSP_SPECTRAL_SOURCES fft_ffts.c)
endif()

add_library(vv-dsp-spectral ${VV_DSP_SPECTRAL_SOURCES})

target_include_directories(vv-dsp-spectral PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<INSTALL_INTERFACE:include>
)

# Link to window module for STFT window functions and core module for NaN policy
target_link_libraries(vv-dsp-spectral PUBLIC vv-dsp-window vv-dsp-core)

# Link FFT backend libraries
if(VV_DSP_WITH_FFTW AND FFTW3F_FOUND)
  target_link_libraries(vv-dsp-spectral PRIVATE fftw3f)
endif()

if(VV_DSP_WITH_FFTS AND FFTS_FOUND)
  target_link_libraries(vv-dsp-spectral PRIVATE ffts)
endif()

if(NOT WIN32)
	find_library(M_MATH m)
	if(M_MATH)
		target_link_libraries(vv-dsp-spectral PUBLIC ${M_MATH})
	endif()
endif()
