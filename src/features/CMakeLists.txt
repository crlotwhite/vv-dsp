add_library(vv-dsp-features
    mel.c
)

target_include_directories(vv-dsp-features PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(vv-dsp-features
    vv-dsp-spectral
    m  # Math library for log functions
)
