# Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT). All rights reserved.
# This software may be modified and distributed under the terms of the
# GNU Lesser General Public License v2.1 or any later version.

yarp_prepare_plugin(xsens_mvn TYPE yarp::dev::XsensMVN
                              INCLUDE XsensMVN.h
                              CATEGORY device
                              ADVANCED
                              DEFAULT ON)

set(PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVN.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNPrivate.cpp"
                   "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNCalibrator.cpp")

set(PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVN.h"
                   "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNPrivate.h"
                   "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNCalibrator.h")

yarp_add_plugin(xsens_mvn ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

#include_directories(${xsens-mvn_SOURCE_DIR}/yarp) #This will be removed

target_link_libraries(xsens_mvn YARP::YARP_OS
                                YARP::YARP_dev
                                YARP::YARP_sig
                                ${XsensXME_LIBRARIES}
                                yarp_experimental)

target_include_directories(xsens_mvn SYSTEM PUBLIC ${YARP_INCLUDE_DIRS})
target_include_directories(xsens_mvn PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

#TODO: check if YARP_XXX is correct as variable (i.e. it consider the current PREFIX)
yarp_install(TARGETS xsens_mvn
             COMPONENT runtime
             LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
             ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR})

yarp_install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf/xsens_mvn.ini"
             DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})
