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
    set(CMAKE_CXX_STANDARD 11)
endif()

# Find YARP and include the CMake code to compile plugins
find_package(YARP REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})

include(YarpPlugin)
include(YarpInstallationHelpers)
set(YARP_FORCE_DYNAMIC_PLUGINS TRUE)
# I want to install plugins in PREFIX/yarp.
#yarp_configure_external_installation(xsens-mvn-driver WITH_PLUGINS)

# Compile the plugins by default
set(COMPILE_BY_DEFAULT ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(yarp_experimental STATIC   
            ${xsens-mvn_SOURCE_DIR}/yarp/include/yarp/dev/IHumanSkeleton.h
            ${xsens-mvn_SOURCE_DIR}/yarp/IHumanSkeleton.cpp)

target_link_libraries(yarp_experimental YARP::YARP_dev)
target_include_directories(yarp_experimental SYSTEM PUBLIC yarp/include ${YARP_INCLUDE_DIRS})
target_include_directories(yarp_experimental PRIVATE yarp/include/yarp/dev)

find_package(XsensXME)

add_subdirectory(xsensdriver)
add_subdirectory(xsenswrapper)
add_subdirectory(xsensremote)
