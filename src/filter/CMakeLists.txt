add_library(vv-dsp-filter
	filter.c
	fir.c
	iir.c
	common.c
)

target_include_directories(vv-dsp-filter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Link dependency on spectral explicitly to resolve fft symbols from vv-dsp-filter
target_link_libraries(vv-dsp-filter PUBLIC vv-dsp-spectral)
