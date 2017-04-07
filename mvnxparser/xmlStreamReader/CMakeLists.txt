# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(xmlStreamReader)

set(CMAKE_CXX_STANDARD 11)

# Load dependencies
find_package(Qt5Xml REQUIRED)
find_package(Qt5XmlPatterns REQUIRED)

# Build libraries
add_subdirectory(xmlMessageHandler)

# Build the library
add_library(xmlStreamReader STATIC xmlStreamReader.cpp) # TODO shared?

# Link the libraries used by this library
target_link_libraries(xmlStreamReader xmlMessageHandler)

# Include directories and export them to who inherits this library
target_include_directories(xmlStreamReader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Use QT Modules
qt5_use_modules(xmlStreamReader Xml XmlPatterns)
