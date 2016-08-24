# - Find portaudio
# Find the native PORTAUDIO includes and library
#
#  PORTAUDIO_INCLUDE_DIRS - where to find portaudio.h, etc.
#  PORTAUDIO_LIBRARIES    - List of libraries when using portaudio.
#  PORTAUDIO_FOUND        - True if portaudio found.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

IF (PORTAUDIO_INCLUDE_DIR)
  # Already in cache, be silent
  SET(PORTAUDIO_FIND_QUIETLY TRUE)
ENDIF (PORTAUDIO_INCLUDE_DIR)

FIND_PATH(PORTAUDIO_INCLUDE_DIR 
		NAMES
				portaudio.h
		PATHS
				include
				/usr/include
				/usr/local/include
				/opt/local/include
				/sw/include
)

IF (CMAKE_CL_64)
    SET(PORTAUDIO_NAMES portaudio_x64)
ELSE()
    SET(PORTAUDIO_NAMES portaudio_x86)
ENDIF()

FIND_LIBRARY(PORTAUDIO_LIBRARY 
		NAMES
				${PORTAUDIO_NAMES}
		PATHS
				lib
				/usr/lib
				/usr/local/lib
				/opt/local/lib
				/sw/lib
)
MARK_AS_ADVANCED( PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR )

# Per-recommendation
SET(PORTAUDIO_INCLUDE_DIRS "${PORTAUDIO_INCLUDE_DIR}")
SET(PORTAUDIO_LIBRARIES    "${PORTAUDIO_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PORTAUDIO DEFAULT_MSG PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIRS)
