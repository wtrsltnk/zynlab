cmake_minimum_required(VERSION 3.8)

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

add_library(zyn.nio STATIC
    AudioOutput.cpp
    AudioOutput.h
    AudioOutputManager.cpp
    AudioOutputManager.h
    Engine.cpp
    Engine.h
    EngineManager.cpp
    EngineManager.h
    MidiInput.cpp
    MidiInput.h
    MidiInputManager.cpp
    MidiInputManager.h
    Nio.cpp
    Nio.h
    NulEngine.cpp
    NulEngine.h
    RtEngine.cpp
    RtEngine.h
    RtError.h
    RtMidi.cpp
    RtMidi.h
    WavEngine.cpp
    WavEngine.h
    )

target_compile_definitions(zyn.nio
    PRIVATE -DOUT_DEFAULT="${DefaultOutput}"
    )
target_compile_definitions(zyn.nio
    PRIVATE -DIN_DEFAULT="${DefaultInput}"
    )

if(JackEnable)
    target_sources(zyn.nio
        PRIVATE
            JackEngine.cpp
            JackEngine.h
        )
    target_include_directories(zyn.nio
        PRIVATE ${JACK_INCLUDE_DIR}
        )
    target_link_libraries(zyn.nio
        ${JACK_LIBRARIES}
        )
endif(JackEnable)

if(PaEnable)
    target_sources(zyn.nio
        PRIVATE
            PaEngine.cpp
            PaEngine.h
        )
    target_include_directories(zyn.nio
        PRIVATE
            ${PORTAUDIO_INCLUDE_DIR}
        )
    target_link_libraries(zyn.nio
        ${PORTAUDIO_LIBRARIES}
        )
    target_compile_definitions(zyn.nio
        PRIVATE -DPORTAUDIO=1
        )
endif()

if(SdlEnable)
    target_sources(zyn.nio
        PRIVATE
            SdlEngine.cpp
            SdlEngine.h
        )
    target_include_directories(zyn.nio
        PRIVATE
            ${SDL2_INCLUDE_DIR}
        )
    target_link_libraries(zyn.nio
        ${SDL2_LIBRARY}
        )
    target_compile_definitions(zyn.nio
        PRIVATE -DSDL2=1
        )
endif()

if(AlsaEnable)
    target_sources(zyn.nio
        PRIVATE
            AlsaEngine.cpp
            AlsaEngine.h
        )
    target_link_libraries(zyn.nio
        ${ASOUND_LIBRARY}
        )
endif()

if(OssEnable)
    target_sources(zyn.nio
        PRIVATE
            OssEngine.cpp
            OssEngine.h
        )
endif()

target_link_libraries(zyn.nio
    ${zyn.common}
    winmm
    )

target_include_directories(zyn.nio
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_compile_definitions(zyn.nio
    PRIVATE
        -D__WINDOWS_MM__
    )

target_compile_features(zyn.nio
    PRIVATE cxx_auto_type
    PRIVATE cxx_lambdas
    PRIVATE cxx_nullptr
    PRIVATE cxx_strong_enums
    )

install(
    TARGETS
        zyn.nio
    EXPORT
        zyn.nio
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.nio_SOURCE_DIR}/AudioOutput.h
        ${zyn.nio_SOURCE_DIR}/AudioOutputManager.h
        ${zyn.nio_SOURCE_DIR}/Engine.h
        ${zyn.nio_SOURCE_DIR}/EngineManager.h
        ${zyn.nio_SOURCE_DIR}/MidiInput.h
        ${zyn.nio_SOURCE_DIR}/MidiInputManager.h
        ${zyn.nio_SOURCE_DIR}/Nio.h
        ${zyn.nio_SOURCE_DIR}/NulEngine.h
        ${zyn.nio_SOURCE_DIR}/RtEngine.h
        ${zyn.nio_SOURCE_DIR}/RtError.h
        ${zyn.nio_SOURCE_DIR}/RtMidi.h
        ${zyn.nio_SOURCE_DIR}/WavEngine.h
    DESTINATION
        "include/zyn/nio"
    )

install(
    EXPORT
        zyn.nio
    FILE
        zyn.nio-config.cmake
    DESTINATION
        "cmake"
    )
