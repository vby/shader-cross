cmake_minimum_required(VERSION 3.11)

project(shader-cross)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
function(Add3rdparyCMake name repo tag)
    FetchContent_Declare(
            ${name}
            GIT_REPOSITORY "${repo}"
            GIT_TAG        "${tag}"
            GIT_PROGRESS   TRUE
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)
        FetchContent_Populate(${name})
        add_subdirectory(${${name}_SOURCE_DIR} ${${name}_BINARY_DIR})
    endif()
endfunction()

set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
Add3rdparyCMake(glslang https://github.com/KhronosGroup/glslang origin/master)
Add3rdparyCMake(spirv-cross https://github.com/KhronosGroup/SPIRV-Cross origin/master)

file(GLOB sources LIST_DIRECTORIES false src/* include/shader_cross/*)
add_library(shader-cross ${sources} "${glslang_SOURCE_DIR}/StandAlone/ResourceLimits.cpp")
target_include_directories(shader-cross PUBLIC include)
target_link_libraries(shader-cross PRIVATE glslang SPIRV spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl)

file(GLOB sources LIST_DIRECTORIES false cmd/*)
add_executable(shader-cross-cmd ${sources})
target_link_libraries(shader-cross-cmd shader-cross)

