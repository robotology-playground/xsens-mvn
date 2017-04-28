# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

option(ENABLE_mvnx_parser "Enable the Qt-based XML / MVNX parser" ON)

if(ENABLE_mvnx_parser)

    add_subdirectory(XMLStreamReader)
    add_subdirectory(MVNXStreamReader)

endif()
