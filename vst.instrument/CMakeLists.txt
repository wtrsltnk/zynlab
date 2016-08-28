cmake_minimum_required(VERSION 2.8)

project(zyn.vst.instrument)

include_directories(
	${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
	${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
	)

set(app_SRCS
	vst.instrument.cpp
	)

add_library(zyn.vst.instrument SHARED
	${app_SRCS}
	)

target_link_libraries(zyn.vst.instrument
	zyn.vst
	zyn.mixer
	zyn.synth
	zyn.fx
	zyn.dsp
	zyn.common
	)
