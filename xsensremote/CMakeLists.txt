# Copyright: (C) 2016 iCub Facility - Fondazione Istituto Italiano di Tecnologia
# Authors: Francesco Romano <francesco.romano@iit.it>
# CopyPolicy: Released under the terms of the GNU LGPL v2.0+

YARP_PREPARE_PLUGIN(xsens_mvn_remote TYPE yarp::dev::XsensMVNRemote
                                     INCLUDE XsensMVNRemote.h
                                     CATEGORY device)

if(ENABLE_xsens_mvn_remote)

  set(PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNRemote.cpp")

  set(PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNRemote.h")

  yarp_add_plugin(xsens_mvn_remote ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})


  target_link_libraries(xsens_mvn_remote YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig xsens_mvn_idl yarp_experimental)
  target_include_directories(xsens_mvn_remote SYSTEM PUBLIC ${YARP_INCLUDE_DIRS})
  target_include_directories(xsens_mvn_remote PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
  
  target_compile_features(xsens_mvn_remote PRIVATE cxx_lambdas)

  yarp_install(TARGETS xsens_mvn_remote
         COMPONENT runtime
         LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
         ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR})
         
  yarp_install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf/xsens_mvn_remote.ini"  DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})

endif()
