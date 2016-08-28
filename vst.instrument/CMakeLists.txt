cmake_minimum_required(VERSION 2.8)

project(zyn.vst.instrument)

if(NOT EXISTS "VST3\ SDK")
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "downloading VST SDK3...")
		file(DOWNLOAD http://www.steinberg.net/sdk_downloads/vstsdk366_27_06_2016_build_61.zip "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "done")
	else()
		message(STATUS "already downloaded VST SDK3")
	endif()
	if(EXISTS "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "extracting VST SDK3...")
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_BINARY_DIR}/vstsdk3.zip"
			)
		message(STATUS "done")
	endif()
else()
	message(STATUS "VST SDK3 found")
endif()

include_directories(
	${CMAKE_BINARY_DIR}/VST3\ SDK/
	${CMAKE_BINARY_DIR}/VST3\ SDK/public.sdk/source/vst2.x/
	)

set(app_SRCS
	vst.instrument.cpp
	${CMAKE_BINARY_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffect.cpp
	${CMAKE_BINARY_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffectx.cpp
#	${CMAKE_BINARY_DIR}/VST3\ SDK/public.sdk/source/vst2.x/vstplugmain.cpp
    )

set(app_HDRS
	)

add_library(zyn.vst.instrument SHARED
	${app_SRCS}
	${app_HDRS}
	)

target_link_libraries(zyn.vst.instrument
	zyn.mixer
	zyn.synth
	zyn.fx
	zyn.dsp
	zyn.common
	)
