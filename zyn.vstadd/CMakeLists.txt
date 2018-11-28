cmake_minimum_required(VERSION 3.8)

project(zyn.vstadd)

add_library(zyn.vstadd SHARED
    vst.instrument.cpp
	)

target_link_libraries(zyn.vstadd
	zyn.vst
	zyn.mixer
	zyn.synth
	zyn.fx
	zyn.dsp
	zyn.common
	)

target_include_directories(zyn.vstadd
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
    )

target_compile_features(zyn.vstadd
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
        cxx_thread_local
    )
