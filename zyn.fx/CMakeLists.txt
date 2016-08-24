cmake_minimum_required(VERSION 2.8)

project(zyn.fx)

set(zyn_fx_SRCS
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
)

set(zyn_fx_HDRS
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

add_library(zyn.fx
    ${zyn_fx_SRCS}
    ${zyn_fx_HDRS}
    )

target_link_libraries(zyn.fx
    zyn.dsp
    zyn.common
    )

install(FILES ${zyn_fx_HDRS} DESTINATION include/zyn/common)
install(TARGETS zyn.fx DESTINATION lib)
