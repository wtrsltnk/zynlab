cmake_minimum_required(VERSION 2.8)

project(zyn.nio)

find_package(PORTAUDIO REQUIRED)
find_package(SDL2)
if (UNIX)
    find_package(ALSA)
    find_package(JACK)
    find_package(OSS)
    find_package(DSSI)
    find_package(LIBLO)
endif (UNIX)

set(AlsaEnable  ${ALSA_FOUND}      CACHE BOOL  "Enable support for Advanced Linux Sound Architecture")
set(JackEnable  ${JACK_FOUND}      CACHE BOOL  "Enable support for JACK Audio Connection toolKit")
set(OssEnable   ${OSS_FOUND}       CACHE BOOL  "Enable support for Open Sound System")
set(PaEnable    ${PORTAUDIO_FOUND} CACHE BOOL  "Enable support for Port Audio System")
set(SdlEnable   ${SDL2_FOUND}      CACHE BOOL  "Enable support for SDL Audio System")
set(DssiEnable  ${DSSI_FOUND}      CACHE BOOL  "Enable DSSI Plugin compilation")
set(LibloEnable ${LIBLO_FOUND}     CACHE BOOL  "Enable Liblo")

set(zyn_nio_SRCS
    Nio.cpp
    AudioOutput.cpp
    AudioOutputManager.cpp
    MidiInput.cpp
    MidiInputManager.cpp
    Engine.cpp
    EngineManager.cpp
    WavEngine.cpp
    NulEngine.cpp
    RtEngine.cpp
    RtMidi.cpp
    )

set(zyn_nio_HDRS
    Nio.h
    AudioOutput.h
    AudioOutputManager.h
    MidiInput.h
    MidiInputManager.h
    Engine.h
    EngineManager.h
    WavEngine.h
    NulEngine.h
    RtEngine.h
    RtMidi.h
    RtError.h
    )

set(zyn_nio_LIB )

add_definitions(-DOUT_DEFAULT="${DefaultOutput}")
add_definitions(-DIN_DEFAULT="${DefaultInput}")

if(JackEnable)
    include_directories(${JACK_INCLUDE_DIR})
    list(APPEND zyn_nio_SRCS JackEngine.cpp)
    list(APPEND zyn_nio_LIB ${JACK_LIBRARIES})
endif(JackEnable)

if(PaEnable)
    include_directories(${PORTAUDIO_INCLUDE_DIR})
    list(APPEND zyn_nio_SRCS PaEngine.cpp)
    list(APPEND zyn_nio_LIB ${PORTAUDIO_LIBRARIES})
    add_definitions(-DPORTAUDIO=1)
endif(PaEnable)

if(SdlEnable)
    include_directories(${SDL2_INCLUDE_DIR})
    list(APPEND zyn_nio_SRCS SdlEngine.cpp)
    list(APPEND zyn_nio_LIB ${SDL2_LIBRARY})
    add_definitions(-DSDL2=1)
endif(SdlEnable)

if(AlsaEnable)
    list(APPEND zyn_nio_SRCS AlsaEngine.cpp)
    list(APPEND zyn_nio_LIB ${ASOUND_LIBRARY})
endif(AlsaEnable)

if(OssEnable)
    list(APPEND zyn_nio_SRCS OssEngine.cpp)
endif(OssEnable)

add_library(zyn.nio STATIC
    ${zyn_nio_SRCS}
    ${zyn_nio_HDRS}
    )

target_link_libraries(zyn.nio
    ${zyn.common}
    ${zyn_nio_LIB}
    winmm
    )

target_include_directories(zyn.nio
    PRIVATE "${CMAKE_SOURCE_DIR}"
    )

target_compile_definitions(zyn.nio
    PRIVATE -D__WINDOWS_MM__
    )

target_compile_features(zyn.nio
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_lambdas
    PRIVATE cxx_strong_enums
    )

install(FILES ${zyn_nio_HDRS} DESTINATION include/zyn/nio COMPONENT develop)
install(TARGETS zyn.nio DESTINATION lib COMPONENT develop)
