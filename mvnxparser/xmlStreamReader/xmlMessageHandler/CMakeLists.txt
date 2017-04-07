# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

# Dependencies
find_package(Qt5XmlPatterns REQUIRED)

# Build the library
add_library(xmlMessageHandler STATIC xmlMessageHandler.cpp)

# Include directories and export them to who inherits this library
target_include_directories(xmlMessageHandler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Use QT Modules
qt5_use_modules(xmlMessageHandler XmlPatterns)
