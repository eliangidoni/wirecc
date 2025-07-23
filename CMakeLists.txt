cmake_minimum_required (VERSION 3.10)
project(wirecc)

include(CTest)

set(wirecc_VERSION_MAJOR 0)
set(wirecc_VERSION_MINOR 1)
set(wirecc_VERSION_PATCH 0)

option(WIRECC_DEBUG "Build with debugging support" OFF)
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)

set(EXTRA_LIBS ${EXTRA_LIBS} m)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_DOCUMENTATION OFF)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -W -Wextra -Wall -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W -Wextra -Wall -pedantic")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

if(CMAKE_BUILD_TYPE STREQUAL "Testing")
  set(BUILD_DOCUMENTATION OFF)
  set(EXTRA_LIBS ${EXTRA_LIBS} gcov)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wextra -Wall -pedantic -pg -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wextra -Wall -pedantic -pg -fprofile-arcs -ftest-coverage")
endif(CMAKE_BUILD_TYPE STREQUAL "Testing")

if(BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)
  #-- Configure the Template Doxyfile for our specific project
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  #-- Add a custom target to run Doxygen when ever the project is built
  add_custom_target (doc
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
                     SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif(BUILD_DOCUMENTATION)

set(WIRECC_HEADER_FILES
    include/wirecc/wirecc.h
)
add_library(WireCC INTERFACE)
target_include_directories(WireCC INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WIRECC_DEBUG ON)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

configure_file ("${PROJECT_SOURCE_DIR}/include/wirecc_config.h.in"
                "${PROJECT_BINARY_DIR}/include/wirecc/wirecc_config.h")

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wirecc, a small C++ library for data encoding/decoding over the network")
set(CPACK_PACKAGE_VENDOR "Elian Gidoni")
set(CPACK_PACKAGE_CONTACT "https://github.com/eliangidoni")
set(CPACK_PACKAGE_VERSION_MAJOR "${wirecc_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${wirecc_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${wirecc_VERSION_PATCH}")
include(CPack)
