add_executable(vv_dsp_dump_fft dump_fft.c)
 target_link_libraries(vv_dsp_dump_fft PRIVATE vv-dsp)

add_executable(vv_dsp_dump_fir dump_fir.c)
 target_link_libraries(vv_dsp_dump_fir PRIVATE vv-dsp)

add_executable(vv_dsp_dump_iir dump_iir.c)
 target_link_libraries(vv_dsp_dump_iir PRIVATE vv-dsp)

add_executable(vv_dsp_dump_stft_roundtrip dump_stft_roundtrip.c)
 target_link_libraries(vv_dsp_dump_stft_roundtrip PRIVATE vv-dsp)

add_executable(vv_dsp_dump_resample dump_resample.c)
 target_link_libraries(vv_dsp_dump_resample PRIVATE vv-dsp)

add_executable(vv_dsp_dump_fir_coeffs dump_fir_coeffs.c)
 target_link_libraries(vv_dsp_dump_fir_coeffs PRIVATE vv-dsp)

add_executable(vv_dsp_dump_dct dump_dct.c)
 target_link_libraries(vv_dsp_dump_dct PRIVATE vv-dsp)
