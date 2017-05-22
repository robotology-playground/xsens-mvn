# Copyright: (C) 2016 iCub Facility - Fondazione Istituto Italiano di Tecnologia
# Authors: Francesco Romano <francesco.romano@iit.it>
# CopyPolicy: Released under the terms of the GNU LGPL v2.0+

cmake_minimum_required(VERSION 3.0)
project(xsens-mvn)

# Add local CMake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (CMAKE_VERSION VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else()
    set(CMAKE_CXX_EXTENSIONS NO)
    set(CMAKE_CXX_STANDARD 11)
endif()

# Find YARP and include the CMake code to compile plugins
find_package(YARP REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(YarpPlugin)
include(YarpInstallationHelpers)
set(YARP_FORCE_DYNAMIC_PLUGINS TRUE)
# I want to install plugins in PREFIX/yarp.
yarp_configure_plugins_installation(xsens)

# Compile the plugins by default
set(COMPILE_BY_DEFAULT ON)
option(BUILD_SHARED_LIBS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(yarp)

find_package(XsensXME)

add_subdirectory(xsensdriver)
add_subdirectory(xsenswrapper)
add_subdirectory(xsensremote)
add_subdirectory(xsensremotelight)
add_subdirectory(mvnxparser)


#### add the "uninstall" target
include(AddUninstallTarget)

