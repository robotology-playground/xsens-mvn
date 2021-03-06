# Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT). All rights reserved.
# This software may be modified and distributed under the terms of the
# GNU Lesser General Public License v2.1 or any later version.

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# must be shared so as to have a successfull dynamic_cast in yarp plugins view
set(yarp_experimental_public_headers include/yarp/dev/IFrameProvider.h
                                     include/yarp/dev/IIMUFrameProvider.h
                                     include/yarp/dev/IXsensMVNInterface.h)
add_library(yarp_experimental SHARED ${yarp_experimental_public_headers}
                                     IFrameProvider.cpp
                                     IIMUFrameProvider.cpp
                                     IXsensMVNInterface.cpp)

target_link_libraries(yarp_experimental YARP::YARP_dev)
target_include_directories(yarp_experimental INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(yarp_experimental PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/yarp/dev>)
target_include_directories(yarp_experimental PUBLIC  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(yarp_experimental PROPERTIES
                                        PUBLIC_HEADER "${yarp_experimental_public_headers}")

install(TARGETS yarp_experimental
        EXPORT YARP_experimental
        COMPONENT runtime
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/yarp/dev)

install(EXPORT YARP_experimental
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YARP_experimental
        NAMESPACE YARP::
)

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/YARP_experimentalConfig.cmake.in
                              ${CMAKE_BINARY_DIR}/YARP_experimentalConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YARP_experimental)

install(FILES ${CMAKE_BINARY_DIR}/YARP_experimentalConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YARP_experimental)
