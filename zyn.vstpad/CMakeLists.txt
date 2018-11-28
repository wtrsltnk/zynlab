cmake_minimum_required(VERSION 3.8)

project(zyn.vstpad)

add_library(zyn.vstpad SHARED
    vst.instrument.cpp
	)

target_link_libraries(zyn.vstpad
	zyn.vst
	zyn.mixer
	zyn.synth
	zyn.fx
	zyn.dsp
	zyn.common
	)

target_include_directories(zyn.vstpad
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
    )
