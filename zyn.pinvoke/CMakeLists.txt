cmake_minimum_required(VERSION 3.12)

project(zyn.pinvoke)

add_library(zyn.pinvoke SHARED
    main.cpp
    )

target_include_directories(zyn.pinvoke
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        include
    )

target_link_libraries(zyn.pinvoke
    ${ZLIB_LIBRARIES}
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.serialization
    zyn.common
    )

target_compile_features(zyn.pinvoke
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    PRIVATE cxx_range_for
    PRIVATE cxx_thread_local
    PRIVATE cxx_std_17
    )
