# Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT). All rights reserved.
# This software may be modified and distributed under the terms of the
# GNU Lesser General Public License v2.1 or any later version.

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(xsens-mvn)

# Add local CMake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

# Find YARP and include the CMake code to compile plugins
find_package(YARP REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(YarpPlugin)
include(YarpInstallationHelpers)
set(YARP_FORCE_DYNAMIC_PLUGINS TRUE)

# Helper to install the plugin in PREFIX/yarp.
yarp_configure_plugins_installation(xsens)

# Compile the plugins by default
set(COMPILE_BY_DEFAULT ON)
option(BUILD_SHARED_LIBS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# YARP experimental interfaces
add_subdirectory(yarp)

# Shared yarp messages thrifts
add_subdirectory(msgs)

if(WIN32)
  option(XSENS_MVN_USE_SDK "Build the driver and the wrapper for the real MVN system using the MVN SDK" OFF)
  if(XSENS_MVN_USE_SDK)
    find_package(XsensXME REQUIRED)
    if(NOT ${XsensXME_FOUND})
      message("XSENS XME not found but option to use it is enabled.")
      return()
    else()
      add_subdirectory(xsensdriver)
      add_subdirectory(xsenswrapper)
    endif()
  endif()
endif()

option(XSENS_MVN_ENABLE_REMOTES "Build multiplatform yarp clients to remotely control the mvn system" ON)
mark_as_advanced(XSENS_MVN_ENABLE_REMOTES)
if(${XSENS_MVN_ENABLE_REMOTES})
  add_subdirectory(xsensremote)
  add_subdirectory(xsensremotelight)
endif()

option(XSENS_MVN_ENABLE_PARSER "Build Qt-based multiplatform yarp-independent parser for XSens .mvnx files" OFF)
if(${XSENS_MVN_ENABLE_PARSER})
  add_subdirectory(mvnxparser)
endif()

#### add the "uninstall" target
include(AddUninstallTarget)

