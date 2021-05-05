cmake_minimum_required(VERSION 3.15)

project(lib.mxml)

add_library(lib.mxml STATIC
    mxml-attr.c
    mxml-entity.c
    mxml-file.c
    mxml-index.c
    mxml-node.c
    mxml-private.c
    mxml-search.c
    mxml-set.c
    mxml-string.c)

target_include_directories(lib.mxml
    PRIVATE vcnet
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(
    TARGETS
        lib.mxml
    EXPORT
        mxml
    DESTINATION
        "lib"
    )


install(
    FILES
        ${mxml_SOURCE_DIR}/mxml.h
    DESTINATION
        "include"
    )

install(
    EXPORT
        mxml
    FILE
        mxml-config.cmake
    DESTINATION
        "cmake"
    )
