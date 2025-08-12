cmake_minimum_required(VERSION 3.15)

project(VVDSPConsumerTest VERSION 1.0.0 LANGUAGES C)

# Find vv-dsp package
find_package(vv-dsp CONFIG REQUIRED)

# Create test executable
add_executable(vv-dsp-consumer-test main.c)

# Link against vv-dsp
target_link_libraries(vv-dsp-consumer-test PRIVATE vv-dsp::vv-dsp)

# Set C standard
set_property(TARGET vv-dsp-consumer-test PROPERTY C_STANDARD 99)
