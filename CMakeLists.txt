cmake_minimum_required(VERSION 3.1.0)
project(wlib-tlsf)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(TLSF_FILES
    include/tlsf.h
    include/wlib/tlsf
    src/config.h
    src/internal.h
    src/tlsf.cpp
    )

add_library(wlib-tlsf ${TLSF_FILES})
target_include_directories(wlib-tlsf PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(wlib-tlsf PRIVATE
    WLIB_TLSF_64BIT
    WLIB_TLSF_LOG2_DIV=5
    WLIB_TLSF_LOG2_ALIGN=3
    WLIB_TLSF_LOG2_MAX=15
    )

file(GLOB_RECURSE TEST_FILES
    ${CMAKE_CURRENT_LIST_DIR}/unit/*.cpp
    )

add_executable(unit ${TEST_FILES})
target_include_directories(unit PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(unit wlib-tlsf)
