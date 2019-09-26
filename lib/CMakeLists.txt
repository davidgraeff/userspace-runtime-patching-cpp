cmake_minimum_required(VERSION 3.14)
project(runtime_patching_lib)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CTest)
find_package(Threads)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE FILES src/*.cpp)
file(GLOB_RECURSE FILES_H src/*.h)

add_library(runtime_patching_lib STATIC ${FILES} ${FILES_H})
target_include_directories(runtime_patching_lib PUBLIC src/include)
target_link_libraries(runtime_patching_lib PUBLIC ${CMAKE_THREAD_LIBS_INIT} -ldl)
set_property(TARGET runtime_patching_lib PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package(Doxygen
        OPTIONAL_COMPONENTS mscgen dia dot)

if (DOXYGEN_FOUND)
    set(DOXYGEN_MARKDOWN_SUPPORT YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ../readme.md)
    doxygen_add_docs(doc
            ${FILES} ${FILES_H} ../readme.md)
endif ()

if (BUILD_TESTING)
    include(AddGoogleTest)

    foreach (test_file IN ITEMS vtable_patch)
        add_executable(VtablePatchTest tests/${test_file}.cpp ${FILES} ${FILES_H})
        target_include_directories(VtablePatchTest PRIVATE src/include)
        target_link_libraries(VtablePatchTest PUBLIC ${CMAKE_THREAD_LIBS_INIT} -ldl)
        add_gtest(VtablePatchTest)
    endforeach ()
endif ()