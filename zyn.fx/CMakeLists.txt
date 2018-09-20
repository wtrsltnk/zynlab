cmake_minimum_required(VERSION 3.8)

project(zyn.fx)

add_library(zyn.fx STATIC
    Alienwah.cpp
    Chorus.cpp
    Distorsion.cpp
    DynamicFilter.cpp
    Echo.cpp
    Effect.cpp
    EffectLFO.cpp
    EffectMgr.cpp
    EQ.cpp
    Phaser.cpp
    Reverb.cpp
    Alienwah.h
    Chorus.h
    Distorsion.h
    DynamicFilter.h
    Echo.h
    Effect.h
    EffectLFO.h
    EffectMgr.h
    EQ.h
    Phaser.h
    Reverb.h
    )

target_include_directories(zyn.fx
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(zyn.fx
    zyn.dsp
    zyn.common
    )

target_compile_features(zyn.fx
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    )

install(
    TARGETS
        zyn.fx
    EXPORT
        zyn.fx
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.fx_SOURCE_DIR}/Alienwah.h
        ${zyn.fx_SOURCE_DIR}/Chorus.h
        ${zyn.fx_SOURCE_DIR}/Distorsion.h
        ${zyn.fx_SOURCE_DIR}/DynamicFilter.h
        ${zyn.fx_SOURCE_DIR}/Echo.h
        ${zyn.fx_SOURCE_DIR}/Effect.h
        ${zyn.fx_SOURCE_DIR}/EffectLFO.h
        ${zyn.fx_SOURCE_DIR}/EffectMgr.h
        ${zyn.fx_SOURCE_DIR}/EQ.h
        ${zyn.fx_SOURCE_DIR}/Phaser.h
        ${zyn.fx_SOURCE_DIR}/Reverb.h
    DESTINATION
        "include/zyn/fx"
    )

install(
    EXPORT
        zyn.fx
    FILE
        zyn.fx-config.cmake
    DESTINATION
        "cmake"
    )
