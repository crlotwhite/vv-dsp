cmake_minimum_required(VERSION 3.15)
project(RegistryTest LANGUAGES C)

find_package(vv-dsp CONFIG REQUIRED)

add_executable(registry_test main.c)
target_link_libraries(registry_test PRIVATE vv-dsp::vv-dsp)
set_property(TARGET registry_test PROPERTY C_STANDARD 99)
