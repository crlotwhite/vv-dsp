add_library(vv-dsp-spectral spectral.c fft.c fft_kiss.c utils.c)

target_include_directories(vv-dsp-spectral PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if(NOT WIN32)
	find_library(M_MATH m)
	if(M_MATH)
		target_link_libraries(vv-dsp-spectral PUBLIC ${M_MATH})
	endif()
endif()
