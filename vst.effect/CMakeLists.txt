cmake_minimum_required(VERSION 2.8)

project(zyn.vst.effect)

include_directories(
	${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/
	${CMAKE_SOURCE_DIR}/zyn.vst/VST3\ SDK/public.sdk/source/vst2.x/
	)

set(app_SRCS
	vst.effect.cpp
    )

add_library(zyn.vst.effect SHARED
	${app_SRCS}
	)

target_link_libraries(zyn.vst.effect
	zyn.vst
	zyn.mixer
	zyn.synth
	zyn.fx
	zyn.dsp
	zyn.common
	)
