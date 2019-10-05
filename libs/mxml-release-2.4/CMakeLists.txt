cmake_minimum_required(VERSION 3.15)

project(mxml)

add_library(mxml
    mxml-attr.c
    mxml-entity.c
    mxml-file.c
    mxml-index.c
    mxml-node.c
    mxml-private.c
    mxml-search.c
    mxml-set.c
    mxml-string.c)

target_include_directories(mxml
    PRIVATE vcnet)

install(
    TARGETS
        mxml
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
