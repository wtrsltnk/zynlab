cmake_minimum_required(VERSION 3.8)

project(zyn.synth)

find_package(FFTW REQUIRED)

add_library(zyn.synth STATIC
    ADnote.cpp
    ADnote.h
    ADnoteParams.cpp
    ADnoteParams.h
    Controller.cpp
    Controller.h
    Envelope.cpp
    Envelope.h
    EnvelopeParams.cpp
    EnvelopeParams.h
    FFTWrapper.cpp
    FFTWrapper.h
    IFFTWrapper.h
    LFO.cpp
    LFO.h
    LFOParams.cpp
    LFOParams.h
    OscilGen.cpp
    OscilGen.h
    PADnote.cpp
    PADnote.h
    PADnoteParams.cpp
    PADnoteParams.h
    Resonance.cpp
    Resonance.h
    SUBnote.cpp
    SUBnote.h
    SUBnoteParams.cpp
    SUBnoteParams.h
    SynthNote.cpp
    SynthNote.h
    fftsg.c
    )

target_include_directories(zyn.synth
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(zyn.synth
    zyn.dsp
    zyn.common
    ${FFTW_LIBRARIES}
    )

target_compile_features(zyn.synth
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_lambdas
    )

install(
    TARGETS
        zyn.synth
    EXPORT
        zyn.synth
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.synth_SOURCE_DIR}/ADnote.h
        ${zyn.synth_SOURCE_DIR}/ADnoteParams.h
        ${zyn.synth_SOURCE_DIR}/Controller.h
        ${zyn.synth_SOURCE_DIR}/Envelope.h
        ${zyn.synth_SOURCE_DIR}/EnvelopeParams.h
        ${zyn.synth_SOURCE_DIR}/FFTWrapper.h
        ${zyn.synth_SOURCE_DIR}/IFFTWrapper.h
        ${zyn.synth_SOURCE_DIR}/LFO.h
        ${zyn.synth_SOURCE_DIR}/LFOParams.h
        ${zyn.synth_SOURCE_DIR}/OscilGen.h
        ${zyn.synth_SOURCE_DIR}/PADnote.h
        ${zyn.synth_SOURCE_DIR}/PADnoteParams.h
        ${zyn.synth_SOURCE_DIR}/Resonance.h
        ${zyn.synth_SOURCE_DIR}/SUBnote.h
        ${zyn.synth_SOURCE_DIR}/SUBnoteParams.h
        ${zyn.synth_SOURCE_DIR}/SynthNote.h
    DESTINATION
        "include/zyn/synth"
    )

install(
    EXPORT
        zyn.synth
    FILE
        zyn.synth-config.cmake
    DESTINATION
        "cmake"
    )
