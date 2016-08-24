# - Find mxml
#
#  MXML_INCLUDE_DIRS - where to find mxml.h, etc.
#  MXML_LIBRARIES    - List of libraries when using mxml.
#  MXML_FOUND        - True if mxml found.

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

IF (MXML_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MXML_FIND_QUIETLY TRUE)
ENDIF (MXML_INCLUDE_DIR)

FIND_PATH(MXML_INCLUDE_DIR 
		NAMES
				mxml.h
		PATHS
				include
				/usr/include
				/usr/local/include
				/opt/local/include
				/sw/include
)

FIND_LIBRARY(MXML_LIBRARY 
		NAMES
				mxml
				mxml1.lib
				mxml1d.lib
		PATHS
				lib
				/usr/lib
				/usr/local/lib
				/opt/local/lib
				/sw/lib
)
MARK_AS_ADVANCED( MXML_LIBRARY MXML_INCLUDE_DIR )

# Per-recommendation
SET(MXML_INCLUDE_DIRS "${MXML_INCLUDE_DIR}")
SET(MXML_LIBRARIES    "${MXML_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set MXML_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MXML DEFAULT_MSG MXML_LIBRARIES MXML_INCLUDE_DIRS)
