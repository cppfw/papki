# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

set(name papki)

file(GLOB_RECURSE srcs "../../src/*.cpp")
file(GLOB_RECURSE deps_srcs "../../src_deps/*.c")

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
        "../../src_deps"
    )

target_link_libraries(
        ${name}
        android log ${ANDROID_GRADLE_NATIVE_MODULES}
    )
