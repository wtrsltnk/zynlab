cmake_minimum_required(VERSION 3.8)

project(zyn.common)

add_library(zyn.common STATIC)

target_sources(zyn.common
    PRIVATE
        AudioFile.cpp
        AudioFile.h
        Config.cpp
        Config.h
        IAudioGenerator.cpp
        IAudioGenerator.h
        IFFTwrapper.h
        ILibraryManager.cpp
        ILibraryManager.h
        IMidiEventHandler.cpp
        IMidiEventHandler.h
        IPresetsSerializer.cpp
        IPresetsSerializer.h
        Presets.cpp
        Presets.h
        PresetsSerializer.cpp
        PresetsSerializer.h
        PresetsStore.cpp
        PresetsStore.h
        Stereo.h
        Util.cpp
        Util.h
        WavData.cpp
        WavData.h
        WavFileWriter.cpp
        WavFileWriter.h
        WaveShapeSmps.cpp
        WaveShapeSmps.h
        base64.cpp
        base64.h
        circularbuffer.h
        dr_wav.h
        filesystemapi.cpp
        filesystemapi.h
        globals.cpp
        globals.h
    )

target_link_libraries(zyn.common
    PUBLIC
        lib.mxml
        zlib
    )

target_include_directories(zyn.common
    PUBLIC
        $<BUILD_INTERFACE:${zlib_SOURCE_DIR}>
        $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
)

target_compile_features(zyn.common
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_std_17
    )

install(
    TARGETS
        zyn.common
        zlib
    EXPORT
        zyn.common
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.common_SOURCE_DIR}/Config.h
        ${zyn.common_SOURCE_DIR}/IAudioGenerator.h
        ${zyn.common_SOURCE_DIR}/Presets.h
        ${zyn.common_SOURCE_DIR}/PresetsStore.h
        ${zyn.common_SOURCE_DIR}/Stereo.h
        ${zyn.common_SOURCE_DIR}/Util.h
        ${zyn.common_SOURCE_DIR}/WavFile.h
        ${zyn.common_SOURCE_DIR}/WavFileReader.h
        ${zyn.common_SOURCE_DIR}/WavFileWriter.h
        ${zyn.common_SOURCE_DIR}/WaveShapeSmps.h
        ${zyn.common_SOURCE_DIR}/XmlWrapper.h
        ${zyn.common_SOURCE_DIR}/base64.h
        ${zyn.common_SOURCE_DIR}/globals.h
    DESTINATION
        "include/zyn/common"
    )

install(
    EXPORT
        zyn.common
    FILE
        zyn.common-config.cmake
    DESTINATION
        "cmake"
    )
