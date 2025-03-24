cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0057 NEW)

project(pymahjong)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")

set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(pymahjong SHARED src/bindings.cpp src/calsht_dw.cpp)

target_include_directories(pymahjong PRIVATE src)

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(pymahjong PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

add_custom_command(TARGET pymahjong POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:pymahjong>/data
)