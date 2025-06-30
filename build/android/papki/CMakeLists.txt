cmake_minimum_required(VERSION 3.4.1)

set(name papki)
project(${name})

file(GLOB_RECURSE srcs "../../../src/*.cpp")
file(GLOB_RECURSE deps_srcs "../../../src_deps/*.c")

include (${ANDROID_GRADLE_NATIVE_BUNDLE_PLUGIN_MK})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_library(
        ${name}
        STATIC
        ${srcs} ${deps_srcs}
    )

target_include_directories(
        ${name}
        PRIVATE
        "../../../src_deps"
    )

target_link_libraries(
        ${name}
        android log ${ANDROID_GRADLE_NATIVE_MODULES}
    )
