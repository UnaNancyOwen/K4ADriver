cmake_minimum_required( VERSION 3.6 )

# Language
enable_language( CXX )

# Compiler Settings
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# Project
project( k4adriver LANGUAGES CXX )
add_library( k4adriver SHARED
  K4ADriver.h
  K4ADriver.cpp
  K4ADevice.h
  K4ADevice.cpp
  K4AStream.h
  K4AStream.cpp
  K4ACapture.h
  K4ACapture.cpp
)

# (Option) Start-Up Project for Visual Studio
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "k4adriver" )

# Find Package
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH} )
find_package( OpenNI2 REQUIRED )
find_package( k4a REQUIRED )
if(NOT WIN32)
  find_package( TBB REQUIRED )
endif()

# Set Package to Project
if( OpenNI2_FOUND AND k4a_FOUND )
  target_link_libraries( k4adriver OpenNI2::OpenNI2 )
  target_link_libraries( k4adriver k4a::k4a )
endif()

if( TBB_FOUND )
  target_link_libraries( k4adriver TBB::tbb )
endif()
