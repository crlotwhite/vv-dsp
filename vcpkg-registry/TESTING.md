# Testing Custom vcpkg Registry

This document provides instructions for testing the vv-dsp custom vcpkg registry.

## Local Testing (Filesystem Registry)

For local testing during development, you can use a filesystem registry:

### 1. Setup Test Project

```bash
mkdir test-registry && cd test-registry
```

### 2. Create vcpkg-configuration.json

```json
{
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/Microsoft/vcpkg",
    "baseline": "2024.01.12"
  },
  "registries": [
    {
      "kind": "filesystem",
      "path": "../vcpkg-registry",
      "packages": [ "vv-dsp" ]
    }
  ]
}
```

### 3. Create vcpkg.json

```json
{
  "dependencies": [
    {
      "name": "vv-dsp",
      "features": ["simd"]
    }
  ]
}
```

### 4. Create CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(RegistryTest LANGUAGES C)

find_package(vv-dsp CONFIG REQUIRED)

add_executable(registry_test main.c)
target_link_libraries(registry_test PRIVATE vv-dsp::vv-dsp)
set_property(TARGET registry_test PROPERTY C_STANDARD 99)
```

### 5. Create main.c

```c
#include <stdio.h>
#include <vv_dsp/vv_dsp.h>

int main(void) {
    printf("Testing vv-dsp from custom registry...\n");

    const size_t N = 64;
    vv_dsp_real window[64];
    vv_dsp_status status = vv_dsp_window_hann(N, window);

    if (status != VV_DSP_OK) {
        printf("FAILED: Could not create window\n");
        return 1;
    }

    printf("SUCCESS: Custom registry integration test passed\n");
    return 0;
}
```

### 6. Build and Test

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/registry_test
```

## Remote Testing (Git Registry)

For testing the actual Git registry:

### 1. Replace vcpkg-configuration.json

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

### 2. Clean and Rebuild

```bash
rm -rf build
rm -rf vcpkg_installed
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Expected Results

A successful test should show:

```
Testing vv-dsp from custom registry...
SUCCESS: Custom registry integration test passed
Window function created with 64 samples
Sample values: window[0]=0.000, window[16]=0.188, window[32]=1.000
```

## Troubleshooting

### Registry Not Found

If you see errors about the registry not being found:

1. Check the `vcpkg-configuration.json` syntax
2. Verify the registry path or URL is correct
3. Ensure the registry contains a `ports/vv-dsp/` directory

### Build Failures

If compilation fails:

1. Make sure the vcpkg toolchain file is specified
2. Check that all required dependencies are available
3. Verify CMake can find the package config files

### Feature Issues

If specific features don't work:

1. Check that features are properly specified in `vcpkg.json`
2. Verify feature dependencies are available in the registry
3. Review the portfile.cmake for correct feature mapping
