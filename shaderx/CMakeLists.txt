cmake_minimum_required(VERSION 3.11)

project(shader-cross-shaderx)

Add3rdparty(cxxopts https://github.com/jarro2783/cxxopts.git v2.1.1 TRUE)

file(GLOB sources LIST_DIRECTORIES FALSE *)
add_executable(shaderx ${sources})
target_link_libraries(shaderx PRIVATE cxxopts shader-cross)
