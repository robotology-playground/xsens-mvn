# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

project(XMLStreamReader VERSION 0.1.0)

message (STATUS "${PROJECT_NAME} version ${${PROJECT_NAME}_VERSION}")

# Project configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Load dependencies
find_package(Qt5 COMPONENTS Xml XmlPatterns REQUIRED)

# Build the libraries
# ===================
add_library(XMLMessageHandler
            XMLMessageHandler/XMLMessageHandler.h
            XMLMessageHandler/XMLMessageHandler.cpp)
add_library(XMLStreamReader XMLStreamReader.h XMLStreamReader.cpp)

# Link the libraries used by this library
target_link_libraries(XMLStreamReader XMLMessageHandler)

# Use QT Modules
qt5_use_modules(XMLMessageHandler XmlPatterns)
qt5_use_modules(XMLStreamReader Xml XmlPatterns)

# Set the include directories
# ===========================

# XMLMessageHandler
set(XMLMessageHandler_BUILD_INCLUDEDIR   ${CMAKE_CURRENT_SOURCE_DIR}/XMLMessageHandler)
set(XMLMessageHandler_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/XMLMessageHandler)
#
target_include_directories(XMLMessageHandler PUBLIC
                           $<BUILD_INTERFACE:${XMLMessageHandler_BUILD_INCLUDEDIR}>
                           $<INSTALL_INTERFACE:${XMLMessageHandler_INSTALL_INCLUDEDIR}>)

# XMLStreamReader
set(XMLStreamReader_BUILD_INCLUDEDIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(XMLStreamReader_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/XMLStreamReader)
#
target_include_directories(XMLStreamReader PUBLIC
                           $<BUILD_INTERFACE:${XMLStreamReader_BUILD_INCLUDEDIR}>
                           $<INSTALL_INTERFACE:${XMLStreamReader_INSTALL_INCLUDEDIR}>)

# Build the test unit executable
# ==============================
add_executable(XMLStreamReaderDriver
               ${CMAKE_CURRENT_SOURCE_DIR}/Test/XMLStreamReaderDriver.cpp)
target_link_libraries(XMLStreamReaderDriver XMLStreamReader)

# Install the libraries
# =====================

# XMLStreamReader
set_target_properties(XMLStreamReader
                      PROPERTIES VERSION ${${PROJECT_NAME}_VERSION}
                                 PUBLIC_HEADER XMLStreamReader.h)
install(TARGETS XMLStreamReader
        EXPORT  XMLStreamReader
        RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xml)

# XMLMessageHandler
set_target_properties(XMLMessageHandler
                      PROPERTIES VERSION ${${PROJECT_NAME}_VERSION}
                                 PUBLIC_HEADER XMLMessageHandler/XMLMessageHandler.h)
install(TARGETS XMLMessageHandler
        EXPORT  XMLStreamReader
        RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xml)

# RPATH Support
# =============
option(ENABLE_RPATH "Enable RPATH for this library" ON)
mark_as_advanced(ENABLE_RPATH)
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
                          DEPENDS ENABLE_RPATH
                          USE_LINK_PATH)

# Export the libraries
# ====================

include(InstallBasicPackageFiles)
set(VARS_PREFIX ${PROJECT_NAME})
set_property(GLOBAL PROPERTY ${VARS_PREFIX}_TARGETS XMLStreamReader XMLMessageHandler)
set_property(GLOBAL PROPERTY {VARS_PREFIX}_BUILD_INCLUDEDIR
                             "${XMLMessageHandler_BUILD_INCLUDEDIR} ${XMLStreamReader_BUILD_INCLUDEDIR}")
set_property(GLOBAL PROPERTY {VARS_PREFIX}_INSTALL_INCLUDEDIR
                             "${XMLMessageHandler_INSTALL_INCLUDEDIR} ${XMLStreamReader_INSTALL_INCLUDEDIR}")

install_basic_package_files(${PROJECT_NAME} VERSION          ${${VARS_PREFIX}_VERSION}
                                            VARS_PREFIX      ${VARS_PREFIX}
                                            COMPATIBILITY    AnyNewerVersion
                                            TARGETS_PROPERTY ${VARS_PREFIX}_TARGETS
                                            NO_CHECK_REQUIRED_COMPONENTS_MACRO
                                            EXTRA_PATH_VARS_SUFFIX )
