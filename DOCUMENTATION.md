# VV-DSP Documentation Guide

## Documentation Generation

VV-DSP uses [Doxygen](https://www.doxygen.nl/) to generate comprehensive API documentation from source code comments.

### Prerequisites

- Doxygen 1.9.x or later
- Optional: Graphviz for generating diagrams

### Generating Documentation

**Online Documentation**: The latest documentation is automatically generated and published at:
**🌐 [https://crlotwhite.github.io/vv-dsp/](https://crlotwhite.github.io/vv-dsp/)**

**Local Generation**: To build the documentation locally:

```bash
# From the project root directory
doxygen Doxyfile
```

This will generate HTML documentation in the `docs/html/` directory. Open `docs/html/index.html` in your web browser to view the documentation.

**Automated Deployment**: Documentation is automatically updated via GitHub Actions when changes are pushed to the main branch. See [.github/PAGES_SETUP.md](.github/PAGES_SETUP.md) for setup details.

### Documentation Structure

The generated documentation includes:

- **Main Page**: Project overview and getting started guide
- **Modules**: Organized by functional groups (Core, Spectral, Filter, etc.)
- **API Reference**: Detailed function documentation with parameters and examples
- **File Reference**: Source file organization and dependencies
- **Examples**: Code examples demonstrating library usage

### Module Organization

The library is organized into the following documented modules:

- **Core** (`core_group`): Basic mathematical operations and statistics
- **Spectral** (`spectral_group`): FFT, STFT, DCT, CZT, and Hilbert transforms
- **Filter** (`filter_group`): FIR, IIR, and Savitzky-Golay filters
- **Resample** (`resample_group`): Sample rate conversion and interpolation
- **Envelope** (`envelope_group`): Signal envelope extraction and analysis
- **Window** (`window_group`): Various windowing functions
- **Features** (`features_group`): MFCC and other feature extraction
- **Audio** (`audio_group`): Audio I/O operations (optional)
- **Adapters** (`adapters_group`): External library integration

### Customizing Documentation

The documentation configuration can be customized by editing the `Doxyfile`:

- **PROJECT_NAME**: Change the project title
- **PROJECT_NUMBER**: Update version number
- **OUTPUT_DIRECTORY**: Change output location
- **INPUT**: Add or remove source directories
- **EXTRACT_ALL**: Set to YES to document all functions (including undocumented ones)

### Online Documentation

For the latest documentation, visit: **[VV-DSP Documentation](https://crlotwhite.github.io/vv-dsp/)**

The online documentation is automatically updated from the main branch and includes all the latest API changes and improvements.

### Contributing to Documentation

When contributing code, please follow these documentation guidelines:

1. Use Javadoc-style comments (`/** ... */`)
2. Include `@brief` descriptions for all public functions
3. Document all parameters with `@param`
4. Document return values with `@return`
5. Add usage examples with `@code{.c} ... @endcode`
6. Use `@note`, `@warning`, and `@see` as appropriate
7. Group related functions using `@addtogroup`

Example documentation format:

```c
/**
 * @brief Compute the magnitude of a complex number
 * @param z Input complex number
 * @return Magnitude |z| = sqrt(re² + im²)
 *
 * @code{.c}
 * vv_dsp_cpx z = vv_dsp_cpx_make(3.0, 4.0);  // 3+4i
 * vv_dsp_real mag = vv_dsp_cpx_abs(z);       // 5.0
 * @endcode
 *
 * @see vv_dsp_cpx_phase(), vv_dsp_cpx_from_polar()
 */
vv_dsp_real vv_dsp_cpx_abs(vv_dsp_cpx z);
```
