cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

project(pymahjong)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(_core MODULE src/bindings.cpp src/calsht_dw.cpp)

target_include_directories(_core PRIVATE src)
target_compile_features(_core PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(_core PRIVATE /W4)
else()
    target_compile_options(_core PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
        -Wall
        -Wextra
    )
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_property(TARGET _core PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(_core PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

add_custom_command(TARGET _core POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:_core>/data
)
