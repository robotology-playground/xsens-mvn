# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

option(ENABLE_mvnx_parser "Enable the Qt-based XML parser" ON)

if(ENABLE_mvnx_parser)

    # Build libraries
    add_subdirectory(xmlStreamReader)
    add_subdirectory(mvnxStreamReader)

    # Build the executables
    # XML
    add_executable(xmlStreamReaderDriver xmlStreamReaderDriver.cpp)
    target_link_libraries(xmlStreamReaderDriver xmlStreamReader)
    # MNVX
    add_executable(mvnxStreamReaderDriver mvnxStreamReaderDriver.cpp)
    target_link_libraries(mvnxStreamReaderDriver mvnxStreamReader)

endif()
