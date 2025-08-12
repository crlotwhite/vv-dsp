add_library(vv-dsp-features
    mel.c
)

target_include_directories(vv-dsp-features PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link math library on non-Windows systems
if(NOT WIN32)
    find_library(M_MATH m)
    if(M_MATH)
        target_link_libraries(vv-dsp-features
            vv-dsp-spectral
            ${M_MATH}
        )
    else()
        target_link_libraries(vv-dsp-features
            vv-dsp-spectral
        )
    endif()
else()
    target_link_libraries(vv-dsp-features
        vv-dsp-spectral
    )
endif()
