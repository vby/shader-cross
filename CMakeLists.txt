cmake_minimum_required(VERSION 3.11)

project(shader-cross)

option(SHADER_CROSS_SHADERX "Build shaderx" ON)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
function(Add3rdparty name repo tag subdir)
    FetchContent_Declare(
            ${name}
            GIT_REPOSITORY "${repo}"
            GIT_TAG        "${tag}"
            GIT_PROGRESS   TRUE
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)
        FetchContent_Populate(${name})
        if (subdir)
            add_subdirectory(${${name}_SOURCE_DIR} ${${name}_BINARY_DIR})
        endif()
    endif()
endfunction()

set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
Add3rdparty(glslang https://github.com/KhronosGroup/glslang master TRUE)
Add3rdparty(spirv-cross https://github.com/KhronosGroup/SPIRV-Cross master TRUE)

file(GLOB sources LIST_DIRECTORIES FALSE src/* include/shader_cross/*)
add_library(shader-cross ${sources} "${glslang_SOURCE_DIR}/StandAlone/ResourceLimits.cpp")
target_include_directories(shader-cross PUBLIC include)
target_link_libraries(shader-cross PRIVATE glslang SPIRV spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl)

if (SHADER_CROSS_SHADERX)
    add_subdirectory(shaderx)
endif()

include(CTest)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
