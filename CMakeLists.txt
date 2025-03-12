cmake_minimum_required(VERSION 3.1)

project(pymahjong)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")

add_subdirectory(pybind11)
pybind11_add_module(pymahjong src/bindings.cpp src/calsht_dw.cpp)

target_include_directories(pymahjong PRIVATE src)

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(pymahjong PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})