#.rst:
# FindOpenNI2
# -----------
#
# Find OpenNI2 SDK include dirs, and libraries.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the :prop_tgt:`IMPORTED` targets:
#
# ``OpenNI2::OpenNI2``
#  Defined if the system has OpenNI2 SDK.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following variables:
#
# ::
#
#   OpenNI2_FOUND               True in case OpenNI2 SDK is found, otherwise false
#   OpenNI2_ROOT                Path to the root of found OpenNI2 SDK installation
#
# Example usage
# ^^^^^^^^^^^^^
#
# ::
#
#     find_package(OpenNI2 REQUIRED)
#
#     add_executable(foo foo.cc)
#     target_link_libraries(foo OpenNI2::OpenNI2)
#
# License
# ^^^^^^^
#
# Copyright (c) 2019 Tsukasa SUGIURA
# Distributed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

set(OPENNI2_SUFFIX)
if(WIN32 AND CMAKE_CL_64)
  set(OPENNI2_SUFFIX 64)
endif()

find_path(OpenNI2_INCLUDE_DIR
  NAMES
    OpenNI.h
  PATHS
    "$ENV{OPENNI2_INCLUDE${OPENNI2_SUFFIX}}"
    "/usr/include"
    "/usr/local/include"
  PATH_SUFFIXES
    openni2
)

find_library(OpenNI2_LIBRARY
  NAMES
    OpenNI2
    libOpenNI2
  PATHS
    "$ENV{OPENNI2_LIB${OPENNI2_SUFFIX}}"
    "/usr/lib"
    "/usr/local/lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpenNI2 DEFAULT_MSG
  OpenNI2_LIBRARY OpenNI2_INCLUDE_DIR
)

if(OpenNI2_FOUND)
  add_library(OpenNI2::OpenNI2 SHARED IMPORTED)

  set_target_properties(OpenNI2::OpenNI2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OpenNI2_INCLUDE_DIR}")
  set_property(TARGET OpenNI2::OpenNI2 APPEND PROPERTY IMPORTED_CONFIGURATIONS "RELEASE")
  set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX")
  if(WIN32)
    set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_IMPLIB_RELEASE "${OpenNI2_LIBRARY}")
  else()
    set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_LOCATION_RELEASE "${OpenNI2_LIBRARY}")
  endif()

  set_property(TARGET OpenNI2::OpenNI2 APPEND PROPERTY IMPORTED_CONFIGURATIONS "DEBUG")
  set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX")
  if(WIN32)
    set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_IMPLIB_DEBUG "${OpenNI2_LIBRARY}")
  else()
    set_target_properties(OpenNI2::OpenNI2 PROPERTIES IMPORTED_LOCATION_DEBUG "${OpenNI2_LIBRARY}")
  endif()

  get_filename_component(OpenNI2_ROOT "${k4a_INCLUDE_DIR}" PATH)
endif()
