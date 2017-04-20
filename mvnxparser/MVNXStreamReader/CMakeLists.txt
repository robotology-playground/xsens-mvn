# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

# Load dependencies
find_package(Qt5Xml REQUIRED)

# Build the libraries
add_library(MVNXStreamReader MVNXStreamReader.cpp)

# Link the libraries used by this library
target_link_libraries(MVNXStreamReader XMLStreamReader)
qt5_use_modules(MVNXStreamReader Xml)

# Export the includes needed to who inherits this library
target_include_directories(MVNXStreamReader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
