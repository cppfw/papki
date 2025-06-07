cmake_minimum_required(VERSION 3.10)

set(name papki)
project(${name})

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

set(srcs)
myci_add_source_files(srcs
    DIRECTORY
        ../../src/${name}
    RECURSIVE
)

set(deps_srcs)
myci_add_source_files(deps_srcs
    DIRECTORY
        ../../src_deps
    RECURSIVE
)

myci_declare_library(${name}
    SOURCES
        ${srcs}
        ${deps_srcs}
    PUBLIC_INCLUDE_DIRECTORIES
        ../../src
    PRIVATE_INCLUDE_DIRECTORIES
        ../../src_deps
    INSTALL_INCLUDE_DIRECTORIES
        ../../src/${name}
    DEPENDENCIES
        utki
        ZLIB
)
