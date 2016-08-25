cmake_minimum_required(VERSION 2.8)

project(zyn.nio)

find_package(PORTAUDIO REQUIRED)

add_definitions(-DPORTAUDIO=1)

SET (AlsaEnable ${ALSA_FOUND} CACHE BOOL
    "Enable support for Advanced Linux Sound Architecture")
SET (JackEnable ${JACK_FOUND} CACHE BOOL
    "Enable support for JACK Audio Connection toolKit")
SET (OssEnable ${OSS_FOUND} CACHE BOOL
    "Enable support for Open Sound System")
SET (PaEnable ${PORTAUDIO_FOUND} CACHE BOOL
    "Enable support for Port Audio System")
SET (LashEnable ${LASH_FOUND} CACHE BOOL
    "Enable LASH Audio Session Handler")
SET (DssiEnable ${DSSI_FOUND} CACHE BOOL
    "Enable DSSI Plugin compilation")
SET (LibloEnable ${LIBLO_FOUND} CACHE BOOL
    "Enable Liblo")

set(zyn_nio_SRCS
    WavEngine.cpp
    NulEngine.cpp
    AudioOut.cpp
    MidiIn.cpp
    OutMgr.cpp
    InMgr.cpp
    Engine.cpp
    EngineMgr.cpp
    Nio.cpp
    )

set(zyn_nio_HDRS
    WavEngine.h
    NulEngine.h
    AudioOut.h
    MidiIn.h
    OutMgr.h
    InMgr.h
    Engine.h
    EngineMgr.h
    Nio.h
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
endif(PaEnable)

if(AlsaEnable)
    list(APPEND zyn_nio_SRCS AlsaEngine.cpp)
    list(APPEND zyn_nio_LIB ${ASOUND_LIBRARY})
endif(AlsaEnable)

if(OssEnable)
    list(APPEND zyn_nio_SRCS OssEngine.cpp)
endif(OssEnable)

add_library(zyn.nio
    ${zyn_nio_SRCS} 
    )

target_link_libraries(zyn.nio
    ${zun.common}
    ${zyn_nio_LIB}
    )

install(FILES ${zyn_nio_HDRS} DESTINATION include/zyn/nio COMPONENT develop)
install(TARGETS zyn.nio DESTINATION lib COMPONENT develop)
