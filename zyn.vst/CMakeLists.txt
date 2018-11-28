cmake_minimum_required(VERSION 3.8)

project(zyn.vst)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK")
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "downloading VST SDK3...")
		file(DOWNLOAD http://www.steinberg.net/sdk_downloads/vstsdk366_27_06_2016_build_61.zip "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "done")
	else()
		message(STATUS "already downloaded VST SDK3")
	endif()
	if(EXISTS "${CMAKE_BINARY_DIR}/vstsdk3.zip")
		message(STATUS "extracting VST SDK3, this may take a while...")
		execute_process(
			COMMAND
			    ${CMAKE_COMMAND} -E tar xzf "${CMAKE_BINARY_DIR}/vstsdk3.zip"
			WORKING_DIRECTORY
			    ${CMAKE_CURRENT_SOURCE_DIR}
			)
		message(STATUS "done")
	endif()
else()
	message(STATUS "VST SDK3 found")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/
	)

set(app_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffect.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffectx.cpp
    )

set(app_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffect.h
	${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffectx.h
	)

add_library(zyn.vst
	${app_SRCS}
	${app_HDRS}
	)
