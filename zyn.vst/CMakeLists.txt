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

add_library(zyn.vst
    vstcontrol.cpp
    vstcontrol.h
    vstknob.cpp
    vstknob.h
    vstplugin.cpp
    vstplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffectx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/audioeffectx.h
    )

target_include_directories(zyn.vst
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/
        ${CMAKE_CURRENT_SOURCE_DIR}/VST3\ SDK/public.sdk/source/vst2.x/
    )

target_compile_features(zyn.vst
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
        cxx_thread_local
    )
