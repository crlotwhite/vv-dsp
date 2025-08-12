# vcpkg portfile for vv-dsp
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO crlotwhite/vv-dsp
    REF "v${VERSION}"
    SHA512 0
    HEAD_REF main
)

# Feature-based option mapping
set(VV_DSP_OPTIONS)

# SIMD optimizations feature
if("simd" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_SIMD=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_SIMD=OFF")
endif()

# Audio I/O feature
if("audio-io" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_ENABLE_AUDIO_IO=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_ENABLE_AUDIO_IO=OFF")
endif()

# FFT backends
if("fftw" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_WITH_FFTW=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_WITH_FFTW=OFF")
endif()

if("ffts" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_WITH_FFTS=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_WITH_FFTS=OFF")
endif()

# Math approximation features
if("fastapprox" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_FASTAPPROX=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_FASTAPPROX=OFF")
endif()

if("math-approx" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_MATH_APPROX=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_USE_MATH_APPROX=OFF")
endif()

# Build configuration
if("benchmarks" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_BENCHMARKS=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_BENCHMARKS=OFF")
endif()

if("examples" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_EXAMPLES=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_EXAMPLES=OFF")
endif()

if("tests" IN_LIST FEATURES)
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_TESTS=ON")
else()
    list(APPEND VV_DSP_OPTIONS "-DVV_DSP_BUILD_TESTS=OFF")
endif()

# Always disable Python verification tests in vcpkg
list(APPEND VV_DSP_OPTIONS "-DVERIFY_WITH_PYTHON=OFF")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${VV_DSP_OPTIONS}
)

vcpkg_cmake_install()

# Fix CMake targets
vcpkg_cmake_config_fixup(
    PACKAGE_NAME "vv-dsp"
    CONFIG_PATH "lib/cmake/vv-dsp"
)

# Remove duplicate includes from debug
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Install license
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

# Install usage file
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
