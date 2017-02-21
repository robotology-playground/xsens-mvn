# Copyright: (C) 2016 iCub Facility - Fondazione Istituto Italiano di Tecnologia
# Authors: Francesco Romano <francesco.romano@iit.it>
# CopyPolicy: Released under the terms of the GNU LGPL v2.0+

if (NOT ${XsensXME_FOUND})
  return()
endif()

YARP_PREPARE_PLUGIN(xsens_mvn TYPE yarp::dev::XsensMVN
                              INCLUDE XsensMVN.h
                              CATEGORY device)

if(ENABLE_xsens_mvn)

  set(PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVN.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNPrivate.cpp"
             "${CMAKE_CURRENT_SOURCE_DIR}/src/XsensMVNCalibrator.cpp")

  set(PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVN.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNPrivate.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/include/XsensMVNCalibrator.h")

  #yarp_add_plugin
  #set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  yarp_add_plugin(xsens_mvn ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
  #add_library(xsens_mvn SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})
  include_directories(${xsens-mvn_SOURCE_DIR}/yarp) #This will be removed

  target_link_libraries(xsens_mvn YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig ${XsensXME_LIBRARIES} yarp_experimental)
  target_include_directories(xsens_mvn SYSTEM PUBLIC ${YARP_INCLUDE_DIRS})
  target_include_directories(xsens_mvn PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

  #TODO: check if YARP_XXX is correct as variable (i.e. it consider the current PREFIX)
  yarp_install(TARGETS xsens_mvn
         COMPONENT runtime
         LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
         ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR})
  yarp_install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/conf/xsens_mvn.ini"  DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})


  ##add_executable(xsens_test src/test.cpp)
  #target_link_libraries(xsens_test YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig)
  #target_include_directories(xsens_test PUBLIC ${YARP_INCLUDE_DIRS})
  #target_link_libraries(xsens_test YARP::YARP_OS YARP::YARP_dev YARP::YARP_sig ${XsensXME_LIBRARIES} xsens_mvn)

endif()
