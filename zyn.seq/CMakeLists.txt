cmake_minimum_required(VERSION 3.8)

project(zyn.seq)

add_library(zyn.seq STATIC
    ArpModes.cpp
    ArpModes.h
    Chords.cpp
    Chords.h
    Sequencer.cpp
    Sequencer.h
    TrackRegion.cpp
    TrackRegion.h
    RegionsManager.cpp
    RegionsManager.h
    NotesGenerator.cpp
    NotesGenerator.h
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
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_thread_local
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
        ${zyn.seq_SOURCE_DIR}/TrackRegion.h
        ${zyn.seq_SOURCE_DIR}/NotesGenerator.h
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
