# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(XMLStreamReader)

# Project configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Load dependencies
find_package(Qt5Xml REQUIRED)
find_package(Qt5XmlPatterns REQUIRED)

# Build libraries
add_subdirectory(XMLMessageHandler)

# Build the library
add_library(XMLStreamReader XMLStreamReader.cpp)

# Link the libraries used by this library
target_link_libraries(XMLStreamReader XMLMessageHandler)

# Include directories and export them to who inherits this library
target_include_directories(XMLStreamReader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Use QT Modules
qt5_use_modules(XMLStreamReader Xml XmlPatterns)
