# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

option(ENABLE_mvnx_parser "Enable the Qt-based XML parser" ON)

if(ENABLE_mvnx_parser)

    # Build libraries
    add_subdirectory(XMLStreamReader)
    add_subdirectory(MVNXStreamReader)

    # Build the executables
    # XML
    add_executable(XMLStreamReaderDriver XMLStreamReaderDriver.cpp)
    target_link_libraries(XMLStreamReaderDriver XMLStreamReader)
    # MNVX
    add_executable(MVNXStreamReaderDriver MVNXStreamReaderDriver.cpp)
    target_link_libraries(MVNXStreamReaderDriver MVNXStreamReader)

endif()
