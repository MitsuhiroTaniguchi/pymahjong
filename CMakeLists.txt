cmake_minimum_required(VERSION 3.1)

project(pymahjong)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")
set(CMAKE_INSTALL_DATADIR share/data)

option(ENABLE_NYANTEN "ENABLE_NYANTEN" off)
option(ENABLE_PARALLEL "ENABLE_PARALLEL" off)
option(FIX_RANDOM_SEED "FIX_RANDOM_SEED" off)

add_library(calsht_dw SHARED src/calsht_dw.cpp)
if(ENABLE_NYANTEN)
  target_compile_definitions(calsht_dw PUBLIC ENABLE_NYANTEN=true)
endif()
set_target_properties(calsht_dw PROPERTIES PUBLIC_HEADER src/calsht_dw.hpp)

install(TARGETS calsht_dw EXPORT CalshtDW)
install(FILES
        ${PROJECT_BINARY_DIR}/index_dw_s.bin
        ${PROJECT_BINARY_DIR}/index_dw_h.bin
        DESTINATION ${CMAKE_INSTALL_DATADIR})
install(EXPORT CalshtDW FILE CalshtDWConfig.cmake DESTINATION share/cmake)

add_executable(mkind src/mkind.cpp)
if(ENABLE_NYANTEN)
  target_compile_definitions(mkind PUBLIC ENABLE_NYANTEN=true)
endif()
if(ENABLE_PARALLEL)
  target_compile_definitions(mkind PUBLIC ENABLE_PARALLEL=true)
  target_link_libraries(mkind tbb)
endif()

add_executable(sample src/sample.cpp)
target_compile_definitions(sample PUBLIC INDEX_FILE_PATH="${PROJECT_BINARY_DIR}")
if(FIX_RANDOM_SEED)
  target_compile_definitions(sample PUBLIC FIX_RANDOM_SEED=true)
endif()
target_link_libraries(sample calsht_dw)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
add_library(pymahjong MODULE src/bindings.cpp src/calsht_dw.cpp)
target_link_libraries(pymahjong PRIVATE pybind11::module calsht_dw)
target_sources(pymahjong PRIVATE src/calsht_dw.cpp)
target_include_directories(pymahjong PRIVATE ${Python_INCLUDE_DIRS})