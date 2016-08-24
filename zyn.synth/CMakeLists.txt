cmake_minimum_required(VERSION 2.8)

project(zyn.synth)

find_package(FFTW REQUIRED)

set(zyn_synth_SRCS
    ADnote.cpp
    ADnoteParams.cpp
    Controller.cpp
    Envelope.cpp
    EnvelopeParams.cpp
    FFTWrapper.cpp
    LFO.cpp
    LFOParams.cpp
    OscilGen.cpp
    PADnote.cpp
    PADnoteParams.cpp
    Resonance.cpp
    SUBnote.cpp
    SUBnoteParams.cpp
    SynthNote.cpp
)

set(zyn_synth_HDRS
    ADnote.h
    ADnoteParams.h
    Controller.h
    Envelope.h
    EnvelopeParams.h
    FFTWrapper.h
    LFO.h
    LFOParams.h
    OscilGen.h
    PADnote.h
    PADnoteParams.h
    Resonance.h
    SUBnote.h
    SUBnoteParams.h
    SynthNote.h
    )

include_directories(${CMAKE_SOURCE_DIR})

add_library(zyn.synth
    ${zyn_synth_SRCS}
    ${zyn_synth_HDRS}
    )

target_link_libraries(zyn.synth
    zyn.dsp
    zyn.common
    ${FFTW_LIBRARIES}
    )

install(FILES ${zyn_synth_HDRS} DESTINATION include/zyn/synth)
install(TARGETS zyn.synth DESTINATION lib)
