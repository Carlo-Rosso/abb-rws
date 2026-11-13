cmake_minimum_required(VERSION 3.16...3.31)

# Read version from the package.xml file.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/package.xml package_xml_str)
if(NOT package_xml_str MATCHES "<version>([0-9]+.[0-9]+.[0-9]+)</version>")
  message(FATAL_ERROR "Could not parse project version from package.xml. Aborting.")
endif()

# At this point we either have a proper version string, or we've errored
# out with a FATAL_ERROR above. So assume CMAKE_MATCH_1 contains our
# package's version.
project(abb_rws VERSION ${CMAKE_MATCH_1} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/third_party")

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

########################
## POCO C++ Libraries ##
########################
# We need at least 1.4.3 because of WebSocket support.
# Crypto and NetSSL are needed for RWS 2.0 (HTTPS).
set(ABB_RWS_POCO_COMPONENTS Foundation Net Util XML Crypto NetSSL)

# Prefer the CMake package config shipped by POCO itself (conda-forge, vcpkg,
# Debian/Ubuntu libpoco-dev, upstream installs): it provides relocatable
# Poco::<Component> imported targets, so no absolute paths end up in the
# exported abb_rws targets. Fall back to the bundled FindPoco module for old
# POCO installations without a CMake config.
find_package(Poco 1.4.3 CONFIG QUIET COMPONENTS ${ABB_RWS_POCO_COMPONENTS})
if(Poco_FOUND)
  set(ABB_RWS_POCO_USE_CONFIG ON)
  set(ABB_RWS_POCO_LINK_LIBRARIES)
  foreach(component IN LISTS ABB_RWS_POCO_COMPONENTS)
    list(APPEND ABB_RWS_POCO_LINK_LIBRARIES Poco::${component})
  endforeach()
  message(STATUS "Found POCO ${Poco_VERSION} (CMake config: ${Poco_DIR})")
else()
  set(ABB_RWS_POCO_USE_CONFIG OFF)
  find_package(Poco 1.4.3 MODULE REQUIRED COMPONENTS ${ABB_RWS_POCO_COMPONENTS})
  set(ABB_RWS_POCO_LINK_LIBRARIES ${Poco_LIBRARIES})
endif()

###########
## Build ##
###########
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build dynamically-linked binaries" ON)
endif()

if(NOT DEFINED CMAKE_BUILD_EXAMPLES)
  option(CMAKE_BUILD_EXAMPLES "Build example applications" OFF)
endif()

set(
  SRC_FILES
    src/rws_client.cpp
    src/rws_common.cpp
    src/rws_interface.cpp
    src/rws_poco_client.cpp
    src/rws_rapid.cpp
    src/rws_state_machine_interface.cpp
)

add_library(${PROJECT_NAME} ${SRC_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(NOT ABB_RWS_POCO_USE_CONFIG)
  # The public headers include POCO headers. For installed consumers the
  # include directories are added again by abb_rwsConfig.cmake.
  target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    "$<BUILD_INTERFACE:${Poco_INCLUDE_DIRS}>"
  )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
  ${ABB_RWS_POCO_LINK_LIBRARIES}
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC "ABB_RWS_STATIC_DEFINE")
endif()

####################
## Build Example  ##
####################
if(CMAKE_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

#############
## Install ##
#############
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
  DIRECTORY cmake/modules/third_party/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CMakePackageConfigHelpers)

# Create the ${PROJECT_NAME}Config.cmake.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY
)

# Create the ${PROJECT_NAME}ConfigVersion.cmake.
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
)

# package.xml is installed to share/<name>/ as expected by ROS tooling
# (rospack, ros2 pkg, RoboStack/vinca).
install(
  FILES
    "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/package.xml"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
)

# Export targets.
export(
  EXPORT export_${PROJECT_NAME}
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
  NAMESPACE ${PROJECT_NAME}::
)

install(
  EXPORT export_${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)
