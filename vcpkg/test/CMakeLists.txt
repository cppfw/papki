cmake_minimum_required(VERSION 3.10)

project(test)

find_package(papki CONFIG REQUIRED)

add_executable(test main.cpp)

target_link_libraries(test PRIVATE papki::papki)
