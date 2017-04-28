# Copyright: (C) 2017 iCub Facility
# Author: Diego Ferigo
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

project(MVNXStreamReader VERSION 0.1.0)

# Load dependencies
find_package(Qt5Xml REQUIRED)

# Build the library
# =================
add_library(MVNXStreamReader MVNXStreamReader.h MVNXStreamReader.cpp)

# Link the libraries used by this library
target_link_libraries(MVNXStreamReader XMLStreamReader)
qt5_use_modules(MVNXStreamReader Xml)

# Set the include directories
# ===========================
set(MVNXStreamReader_BUILD_INCLUDEDIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(MVNXStreamReader_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/MVNXStreamReader)
target_include_directories(MVNXStreamReader PUBLIC
                           $<BUILD_INTERFACE:${MVNXStreamReader_BUILD_INCLUDEDIR}>
                           $<INSTALL_INTERFACE:${MVNXStreamReader_INSTALL_INCLUDEDIR}>)

# Build the test unit executable
# ==============================
add_executable(MVNXStreamReaderDriver ${CMAKE_CURRENT_SOURCE_DIR}/Test/MVNXStreamReaderDriver.cpp)
target_link_libraries(MVNXStreamReaderDriver MVNXStreamReader)

# Install the library
# ===================
set_target_properties(MVNXStreamReader PROPERTIES PUBLIC_HEADER MVNXStreamReader.h)
install(TARGETS MVNXStreamReader
        EXPORT MVNXStreamReader
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mvnx)

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
set_property(GLOBAL PROPERTY ${VARS_PREFIX}_TARGETS MVNXStreamReader)
set_property(GLOBAL PROPERTY {VARS_PREFIX}_BUILD_INCLUDEDIR   ${MVNXStreamReader_BUILD_INCLUDEDIR})
set_property(GLOBAL PROPERTY {VARS_PREFIX}_INSTALL_INCLUDEDIR ${MVNXStreamReader_INSTALL_INCLUDEDIR}})

install_basic_package_files(${PROJECT_NAME} VERSION          ${${VARS_PREFIX}_VERSION}
                                            VARS_PREFIX      ${VARS_PREFIX}
                                            COMPATIBILITY    AnyNewerVersion
                                            TARGETS_PROPERTY ${VARS_PREFIX}_TARGETS
                                            NO_CHECK_REQUIRED_COMPONENTS_MACRO
                                            EXTRA_PATH_VARS_SUFFIX )
