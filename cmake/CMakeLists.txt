cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)

set(name papki)

project(${name})

find_package(utki CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE srcs "../src/${name}/*.cpp")
file(GLOB_RECURSE deps_srcs "../src_deps/minizip/*.c")

add_library(
    ${name}
    STATIC
    ${srcs}
    ${deps_srcs}
)

target_compile_features(${name} PUBLIC cxx_std_17)
set_target_properties(${name} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${name} PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(
    ${name}
    PRIVATE
        "../src_deps"
)

target_link_libraries(
    ${name}
    PUBLIC
        utki::utki
)

target_link_libraries(
    ${name}
    PRIVATE
        ZLIB
)

# install library header files preserving directory hierarchy
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../src/${name}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    TARGETS ${name}
    EXPORT ${name}-config # generate cmake configs
)

# install cmake configs
install(EXPORT ${name}-config
    FILE ${name}-config.cmake
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${name}
    NAMESPACE ${name}::
)
