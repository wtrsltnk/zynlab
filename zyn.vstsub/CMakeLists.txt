cmake_minimum_required(VERSION 3.8)

project(zyn.vstsub)

add_library(zyn.vstsub SHARED
    vst.instrument.cpp
	)

target_link_libraries(zyn.vstsub
    zyn.common
	zyn.vst
	zyn.mixer
	zyn.synth
	zyn.fx
    zyn.dsp
	)

target_include_directories(zyn.vstsub
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
        ${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
    )
