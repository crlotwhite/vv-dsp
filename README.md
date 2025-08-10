# vv-dsp

Digital Signal Processing (DSP) building blocks in C. Modular, portable, and testable.

## Build

```sh
cmake -S . -B build
cmake --build build -j
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Options

- `-DVV_DSP_BUILD_TESTS=ON|OFF` (default ON)
- `-DVV_DSP_USE_SIMD=ON|OFF` (default OFF)
- `-DVV_DSP_BACKEND_FFT=none|fftw|kissfft` (default `none`)
- `-DVV_DSP_SINGLE_FILE=ON|OFF` (default OFF)

## License

MIT
