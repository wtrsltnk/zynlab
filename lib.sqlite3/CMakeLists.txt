cmake_minimum_required(VERSION 3.15)

project(lib.sqlite)
    
add_library(lib.sqlite3 STATIC
    src/sqlite3.c)

install(
    TARGETS
        lib.sqlite3
    EXPORT
        sqlite3
    DESTINATION
        "lib"
    )


install(
    FILES
        ${sqlite3_SOURCE_DIR}/include/sqlite3.h
    DESTINATION
        "include"
    )

install(
    EXPORT
        sqlite3
    FILE
        sqlite3-config.cmake
    DESTINATION
        "cmake"
    )
