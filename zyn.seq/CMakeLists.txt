cmake_minimum_required(VERSION 2.8)

project(zyn.seq)

add_library(zyn.seq STATIC
    Sequencer.cpp
    Sequencer.h
    )

target_include_directories(zyn.seq
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(zyn.seq
    zyn.common
    zyn.nio
    )

target_compile_features(zyn.seq
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_thread_local
    )

install(
    TARGETS
        zyn.seq
    EXPORT
        zyn.seq
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.seq_SOURCE_DIR}/Sequencer.h
        ${zyn.seq_SOURCE_DIR}/SequencerStep.h
        ${zyn.seq_SOURCE_DIR}/SequencerStrip.h
    DESTINATION
        "include/zyn/seq"
    )

install(
    EXPORT
        zyn.seq
    FILE
        zyn.seq-config.cmake
    DESTINATION
        "cmake"
    )
