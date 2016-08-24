# - Find fftw
# Find the native FFTW includes and library
#
#  FFTW_INCLUDE_DIRS - where to find fftw3.h, etc.
#  FFTW_LIBRARIES    - List of libraries when using fftw.
#  FFTW_FOUND        - True if fftw found.

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

FIND_PATH(FFTW_INCLUDE_DIR
		NAMES
				fftw3.h
		PATHS
				include
				/usr/include
				/usr/local/include
				/opt/local/include
				/sw/include
)

FIND_LIBRARY(FFTW_LIBRARY 
		NAMES 
				fftw3
				fftw3-3
				libfftw
				fftw.lib
				fftwd.lib
				libfftw3-3.lib
		PATHS
				lib
				/usr/lib
				/usr/local/lib
				/opt/local/lib
				/sw/lib
)
MARK_AS_ADVANCED( FFTW_LIBRARY FFTW_INCLUDE_DIR )

# Per-recommendation
SET(FFTW_INCLUDE_DIRS "${FFTW_INCLUDE_DIR}")
SET(FFTW_LIBRARIES    "${FFTW_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW DEFAULT_MSG FFTW_LIBRARIES FFTW_INCLUDE_DIRS)
