# Copyright: (C) 2016 iCub Facility - Fondazione Istituto Italiano di Tecnologia
# Authors: Luca Tagliapietra <luca.tagliapietra@iit.it>
#          Francesco Romano <francesco.romano@iit.it>
# CopyPolicy: Released under the terms of the GNU LGPL v2.0+

yarp_prepare_plugin(xsens_mvn_wrapper TYPE yarp::dev::XsensMVNWrapper
                                      INCLUDE XsensMVNWrapper.h
                                      CATEGORY device
                                      ADVANCED
                                      DEFAULT ON)

set(PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNWrapper.cpp")
set(PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNWrapper.h")

yarp_add_plugin(xsens_mvn_wrapper ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

target_link_libraries(xsens_mvn_wrapper YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig xsens_mvn_idl yarp_experimental)
target_include_directories(xsens_mvn_wrapper SYSTEM PUBLIC ${YARP_INCLUDE_DIRS})
target_include_directories(xsens_mvn_wrapper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_features(xsens_mvn_wrapper PRIVATE cxx_lambdas)

yarp_install(TARGETS xsens_mvn_wrapper
             COMPONENT runtime
             LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
             ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR})

yarp_install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf/xsens_mvn_wrapper.ini"
             DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})
