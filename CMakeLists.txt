cmake_minimum_required(VERSION 3.1.0)
project(wlib-tlsf)

set(FILES
    include/tlsf.h
    src/tlsf.cpp
    src/internal.h
    src/main.cpp
    )

add_executable(wlib-tlsf ${FILES})
target_include_directories(wlib-tlsf PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
