cmake_minimum_required(VERSION 3.12)

project(zyn.pinvoke)

include(CTest)

add_library(zyn.pinvoke SHARED
    main.cpp
    par.cpp
    par.h
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
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
        cxx_thread_local
        cxx_std_17
    )

file(COPY "${fftw3_SOURCE_DIR}/libfftw3-3.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (EXISTS "${zlib_BINARY_DIR}/zlibd.dll")
    file(COPY "${zlib_BINARY_DIR}/zlibd.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    file(COPY "${zlib_BINARY_DIR}/zlib.dll" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()
