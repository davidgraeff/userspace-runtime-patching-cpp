cmake_minimum_required(VERSION 3.14)
project(runtime_patching)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

add_executable(runtime_patching src/main.cpp)
target_link_libraries(runtime_patching runtime_patching_lib)
target_compile_options(runtime_patching PRIVATE -fno-exceptions -frtti)
set_property(TARGET runtime_patching PROPERTY POSITION_INDEPENDENT_CODE ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 9.0)
    target_compile_options(runtime_patching PRIVATE -flive-patching=inline-only-static)
endif()

project(p1)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/registry)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/registry)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/registry)
add_library(p1 SHARED registry/p1.cpp)
target_compile_options(p1 PRIVATE -fno-exceptions -fno-rtti)
set_target_properties(p1 PROPERTIES PREFIX "")
set_property(TARGET p1 PROPERTY POSITION_INDEPENDENT_CODE ON)