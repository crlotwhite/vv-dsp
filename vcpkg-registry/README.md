# vv-dsp vcpkg Registry

This repository serves as a custom vcpkg registry for the [vv-dsp](https://github.com/crlotwhite/vv-dsp) digital signal processing library.

## Quick Start

To use this registry in your project, create or update your `vcpkg-configuration.json`:

```json
{
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/Microsoft/vcpkg",
    "baseline": "2024.01.12"
  },
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/crlotwhite/vv-dsp-vcpkg-registry",
      "baseline": "main",
      "packages": [ "vv-dsp" ]
    }
  ]
}
```

Then install vv-dsp:

```bash
vcpkg install vv-dsp
```

## Available Features

- `fftw`: Enable FFTW3 backend for FFT operations
- `ffts`: Enable FFTS backend for FFT operations
- `simd`: Enable SIMD optimizations (SSE4.1, AVX2, NEON)
- `audio-io`: Enable WAV audio file I/O utilities
- `fastapprox`: Enable fast math approximations
- `math-approx`: Enable DSP-optimized math approximations
- `benchmarks`: Build performance benchmarks
- `examples`: Build example applications
- `tests`: Build test suite

Example with features:

```bash
vcpkg install vv-dsp[fftw,simd]
```

## Repository Structure

```
ports/
├── vv-dsp/
│   ├── vcpkg.json      # Port manifest
│   ├── portfile.cmake  # Build script
│   └── usage           # Usage instructions
└── vcpkg-configuration.json  # Example configuration
```

## License

This registry is licensed under the same terms as vv-dsp (MIT License).

## Contributing

For issues with the vv-dsp library itself, please visit the [main repository](https://github.com/crlotwhite/vv-dsp).

For issues specific to this vcpkg port, please open an issue in this repository.
