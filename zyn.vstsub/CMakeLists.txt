cmake_minimum_required(VERSION 3.8)

project(zyn.vstsub)

add_library(zyn.vstsub SHARED
    vst.editor.cpp
    vst.instrument.cpp
    vst.instrument.h
	)

target_link_libraries(zyn.vstsub
	zyn.synth
	zyn.fx
    zyn.dsp
    zyn.common
    zyn.vst
	)

target_include_directories(zyn.vstsub
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
    )

target_compile_features(zyn.vstsub
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
        cxx_thread_local
    )
