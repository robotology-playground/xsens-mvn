# Copyright: (C) 2016 iCub Facility - Fondazione Istituto Italiano di Tecnologia
# Authors: Francesco Romano <francesco.romano@iit.it>
# CopyPolicy: Released under the terms of the GNU LGPL v2.0+

YARP_PREPARE_PLUGIN(xsens_mvn_remote_light TYPE yarp::dev::XsensMVNRemoteLight
                                           INCLUDE XsensMVNRemoteLight.h
                                           CATEGORY device)

if(ENABLE_xsens_mvn_remote_light)

  set(PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNRemoteLight.cpp")

  set(PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNRemoteLight.h")

  yarp_add_plugin(xsens_mvn_remote_light ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})


  target_link_libraries(xsens_mvn_remote_light YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig xsens_mvn_idl yarp_experimental)
  target_include_directories(xsens_mvn_remote_light SYSTEM PUBLIC ${YARP_INCLUDE_DIRS})
  target_include_directories(xsens_mvn_remote_light PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
  
  target_compile_features(xsens_mvn_remote_light PRIVATE cxx_lambdas)

  yarp_install(TARGETS xsens_mvn_remote_light
               COMPONENT runtime
               LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
               ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR})

  yarp_install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf/xsens_mvn_remote_light.ini"  DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})

endif()
