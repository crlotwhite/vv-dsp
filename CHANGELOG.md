# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Add default user entity with communication preferences and project details
- Add FFT backend tests to validate availability and functionality
- Add vv-dsp port to vcpkg registry
- Implement fallback scalar math operations and remove C++ IIRFilter implementation
- Add vectorized math operations and benchmarking
- Add IIRFilter C++ wrapper and comprehensive tests
- Add SIMD optimization and aligned memory management
- Improve backend initialization for FFT management with thread-safe checks
- Add FFT backend management and support for FFTW and FFTS
- Add denormal processing benchmarks and floating-point environment control
- Implement comprehensive STFT benchmarking and debugging utilities
- Update benchmark tasks status and refine implementation details
- Refactor NaN/Inf handling in DSP functions for improved clarity and consistency
- Implement NaN/Inf handling policies for numerical stability in DSP operations
- Update performance benchmark task status to deferred and refine benchmark details
- Add benchmarking framework for STFT and timing utilities
- Implement signal framing and overlap-add functions
- Add support for external dependencies and fast approximation libraries
- Add memory management guidelines and configuration for memory tools
- Update CMake configuration and improve mathematical operations in DSP modules
- Add Mel scale and MFCC feature extraction support
- Implement additional window functions and their tests
- Add WAV audio file I/O utilities to vv-dsp
- Add MSVC compatibility guidelines and update test tasks for compatibility checks
- Implement Savitzky–Golay filter with associated tests and CMake configuration
- Enhance CMake configuration and improve code consistency across multiple files
- Update .gitignore and CMakeLists.txt to improve build output management and test command consistency
- Implement Hilbert transform utilities and corresponding tests
- Add task complexity report and enhance task definitions for window functions and audio utilities
- Update task metadata and enhance test_stats.py for CLI argument support
- Implement signal statistics utilities with tests and CLI tool
- Use enum constants for array sizes in CZT tests for better compatibility
- Improve timing function for CZT benchmark on Windows
- Update .gitignore to include Python-related files
- Implement Chirp Z-Transform (CZT) with API, tests, and benchmarks
- Implement Discrete Cosine Transform (DCT-II/III) and DCT-IV with tests and tools
- Add Windows support for Python dependencies installation and validation tests
- Refactor CI workflow to separate build and test steps for Windows and macOS/Linux
- Update CI workflow to support multiple OS builds and sanitization options
- Enhance Python validation tests with tolerance and verbosity controls
- Enable Python-based verification tests with NumPy/SciPy integration and update documentation
- Add Python-based verification harness and tools for DSP testing
- Add Python-based verification harness with NumPy/SciPy integration and CI plumbing for cross-validation tests
- Add AddressSanitizer and UBSanitizer options in CMake, and implement micro-benchmark for FFT and STFT performance
- Add C++ RAII wrapper and corresponding tests for FFT, IIR, and math utilities
- Add envelope module with cepstrum, min-phase, and LPC functionalities, including tests
- Implement FIR and IIR filter modules with associated tests and examples
- Change constant definition for sample size in resampler test to enum for MSVC compatibility
- Refactor math constants usage across resampling and window functions for consistency
- Implement resampling functionality with linear and cubic interpolation, including tests and CMake integration
- Update STFT/ISTFT test to use compile-time constants for improved compatibility
- Implement STFT/ISTFT functionality with tests and update task statuses
- Add new audio files and corresponding oto.ini configuration
- Implement window functions and their validation tests
- Refactor FFT test cases to use defined constant for size and improve readability
- Enhance FFT implementation with error handling and include spectral utilities
- Implement FFT functionality and spectral utilities with tests
- Add math library linking for Unix systems and optimize polar conversion function
- Implement core DSP functions and add unit tests for basic operations
- Initial CMake setup and adding basic DSP modules

### Changed

- Update README for clarity and detail on library features and usage
- Update .gitignore to include .venv and .cache directories
- Add comprehensive repository information and structure details to documentation
- Update CI workflow
- Remove unnecessary whitespace in backend management functions
- Add new entities for code analysis, todo list, and performance optimization opportunities
- Enhance documentation and structure of VV-DSP library headers
- Remove unnecessary blank lines
- Remove outdated configuration and guidelines for Windsurf rules and task-master-ai settings
- Remove unnecessary blank lines in window function implementations and tests
- Format code for consistency and readability in WAV I/O functions and tests
- Replace size_t with enum for array size definitions in envelope tests
- FIR filtering to use FFT-based approach and improve test accuracy
- Filter example and tests to use enum for size definitions
- Init project

### Fixed

- Update GitHub Actions permissions for release workflow
- Increase tolerance for SIMD precision tests
- Update deprecated GitHub Actions in release workflow
- Resolve FFTW function name conflict
- Remove unnecessary blank lines in FFT backend tests and CI configuration
- Cast to vv_dsp_real in czt.c to ensure correct type usage and prevent potential issues
- Improve error message formatting in IIRFilter coefficient validation
- Remove unnecessary blank lines in MSVC compatibility instructions and test files
- Define M_PI in dump_hilbert.c for compatibility and add MSVC compile definitions in CMakeLists.txt
- Ensure M_PI is defined in hilbert_tests.c for compatibility
- Remove unnecessary whitespace in reset methods of STFTProcessor and Resampler classes
- Remove duplicate include and improve LPC magnitude calculation for clarity


---

*This changelog is automatically updated during releases.*
