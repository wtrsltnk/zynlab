cmake_minimum_required(VERSION 3.8)

project(zyn.serialization)

find_package(system.io REQUIRED)

add_library(zyn.serialization STATIC
    ADnoteParamsSerializer.cpp
    ADnoteParamsSerializer.h
    ADnoteVoiceParamSerializer.cpp
    ADnoteVoiceParamSerializer.h
    BankManager.cpp
    BankManager.h
    ControllerSerializer.cpp
    ControllerSerializer.h
    EffectMgrSerializer.cpp
    EffectMgrSerializer.h
    EnvelopeParamsSerializer.cpp
    EnvelopeParamsSerializer.h
    FilterParamsSerializer.cpp
    FilterParamsSerializer.h
    LFOParamsSerializer.cpp
    LFOParamsSerializer.h
    Library.cpp
    LibraryItem.cpp
    LibraryManager.cpp
    LibraryManager.h
    MicrotonalSerializer.cpp
    MicrotonalSerializer.h
    MixerSerializer.cpp
    MixerSerializer.h
    OscilGenSerializer.cpp
    OscilGenSerializer.h
    PADnoteParamsSerializer.cpp
    PADnoteParamsSerializer.h
    ResonanceSerializer.cpp
    ResonanceSerializer.h
    SMPLnoteParamsSerializer.cpp
    SMPLnoteParamsSerializer.h
    SUBnoteParamsSerializer.cpp
    SUBnoteParamsSerializer.h
    SaveToFileSerializer.cpp
    SaveToFileSerializer.h
    TrackSerializer.cpp
    TrackSerializer.h
    )

target_include_directories(zyn.serialization
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(zyn.serialization
    PUBLIC zyn.common
    PRIVATE system.io
    )

target_compile_features(zyn.serialization
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_thread_local
    )

install(
    TARGETS
        zyn.serialization
    EXPORT
        zyn.serialization
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.serialization_SOURCE_DIR}/ADnoteParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/ADnoteParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/ADnoteVoiceParamSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/ADnoteVoiceParamSerializer.h
        ${zyn.serialization_SOURCE_DIR}/ControllerSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/ControllerSerializer.h
        ${zyn.serialization_SOURCE_DIR}/EffectMgrSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/EffectMgrSerializer.h
        ${zyn.serialization_SOURCE_DIR}/EnvelopeParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/EnvelopeParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/FilterParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/FilterParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/LFOParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/LFOParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/MicrotonalSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/MicrotonalSerializer.h
        ${zyn.serialization_SOURCE_DIR}/MixerSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/MixerSerializer.h
        ${zyn.serialization_SOURCE_DIR}/OscilGenSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/OscilGenSerializer.h
        ${zyn.serialization_SOURCE_DIR}/PADnoteParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/PADnoteParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/ResonanceSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/ResonanceSerializer.h
        ${zyn.serialization_SOURCE_DIR}/SUBnoteParamsSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/SUBnoteParamsSerializer.h
        ${zyn.serialization_SOURCE_DIR}/TrackSerializer.cpp
        ${zyn.serialization_SOURCE_DIR}/TrackSerializer.h
    DESTINATION
        "include/zyn/serialization"
    )

install(
    EXPORT
        zyn.serialization
    FILE
        zyn.serialization-config.cmake
    DESTINATION
        "cmake"
    )
