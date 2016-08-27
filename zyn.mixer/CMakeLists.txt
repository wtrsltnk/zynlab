cmake_minimum_required(VERSION 2.8)

project(zyn.mixer)

set(zyn_mixer_SRCS
    Bank.cpp
    Dump.cpp
    Mixer.cpp
    Microtonal.cpp
    Instrument.cpp
    Recorder.cpp
)

set(zyn_mixer_HDRS
    Bank.h
    Dump.h
    Mixer.h
    Microtonal.h
    Instrument.h
    Recorder.h
    )

include_directories(${CMAKE_SOURCE_DIR})

add_library(zyn.mixer
    ${zyn_mixer_SRCS}
    ${zyn_mixer_HDRS}
    )

target_link_libraries(zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.common
    )

install(FILES ${zyn_mixer_HDRS} DESTINATION include/zyn/mixer COMPONENT develop)
install(TARGETS zyn.mixer DESTINATION lib COMPONENT develop)
