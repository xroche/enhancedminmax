PROJECT(toy CXX)
cmake_minimum_required(VERSION 3.8)

# Clang compiler
# --------------

if (NOT DEFINED ENV{CC} AND NOT DEFINED ENV{CXX})
    # Use the clang compiler by default and prefer version 8.
    find_program(CLANG_8_FOUND "clang-8")
    find_program(CLANG_XX_8_FOUND "clang++-8")

    if (CLANG_8_FOUND AND CLANG_XX_8_FOUND)
        set(CMAKE_C_COMPILER "clang-8")
        set(CMAKE_CXX_COMPILER "clang++-8")
    else()
        set(CMAKE_C_COMPILER "clang")
        set(CMAKE_CXX_COMPILER "clang++")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(toy
    minmax.cpp
    minmax.h
)
