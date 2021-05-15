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

add_executable(testpar
    test/testpar.cpp
#    test/catch_amalgamated.cpp
    )

target_include_directories(testpar
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        include
    )

target_link_libraries(testpar
    zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.serialization
    zyn.common
    zyn.pinvoke
    )

add_test(NAME testpar COMMAND testpar)
