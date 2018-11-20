cmake_minimum_required(VERSION 3.11)

project(shader-cross-tests)

set(BUILD_GTEST ON CACHE BOOL "")
set(BUILD_GMOCK OFF CACHE BOOL "")
set(gtest_force_shared_crt ON CACHE BOOL "")
Add3rdparty(googletest https://github.com/google/googletest.git release-1.8.1 TRUE)

file(GLOB sources LIST_DIRECTORIES FALSE *)
add_executable(glsl_tests ${sources})
target_link_libraries(glsl_tests PRIVATE shader-cross gtest gtest_main)
add_test(NAME glsl_tests COMMAND glsl_tests)
