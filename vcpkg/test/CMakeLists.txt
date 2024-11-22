cmake_minimum_required(VERSION 3.10)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(test)

find_package(utki CONFIG REQUIRED)
find_package(papki CONFIG REQUIRED)

add_executable(test main.cpp)

target_link_libraries(test PRIVATE papki::papki)
