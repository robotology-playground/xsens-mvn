# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

# Load dependencies
find_package(Qt5Xml REQUIRED)

# Build the libraries
add_library(mvnxStreamReader mvnxStreamReader.cpp)

# Link the libraries used by this library
target_link_libraries(mvnxStreamReader xmlStreamReader)
qt5_use_modules(mvnxStreamReader Xml)

# Export the includes needed to who inherits this library
target_include_directories(mvnxStreamReader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
