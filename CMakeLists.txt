CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

# This needs to be done before any languages are enabled or
# projects are created.

# includes
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# package information
SET(PACKAGE_NAME      "CookieMapping")
SET(PACKAGE_VERSION   "0.0.1-dev")
SET(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
SET(PACKAGE_BUGREPORT "issues")


#SET(CMAKE_C_COMPILER "gcc-5")
#SET(CMAKE_CXX_COMPILER "g++-5")

PROJECT(${PACKAGE_NAME} C CXX)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${PACKAGE_NAME})
#MARK_AS_ADVANCED(CLEAR CMAKE_INSTALL_PREFIX)

# Check architecture OS
IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  MESSAGE(FATAL_ERROR "requires a 64bit OS")
ENDIF()


# 3rd party library
IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third-party/CMakeLists.txt")
  MESSAGE(FATAL_ERROR "third-party/CMakeLists.txt missing. "
                      "Try updating your submodule with:
rm -r third-party
git submodule update --init --recursive
")
ENDIF()

INCLUDE(CheckFunctionExists)

SET(PROJECT_HOME ${CMAKE_CURRENT_SOURCE_DIR})
SET(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
SET(ETC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/etc")
SET(SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
SET(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
SET(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
SET(TP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/third-party")

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/CMake/Functions.cmake")
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/CMake/FindLibs.cmake")

include_directories("${TP_DIR}")

ADD_SUBDIRECTORY(third-party)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(logger)
#cmake -Dtest=on ..
# ctest -v OR make test
option(test "Build all tests." OFF) 
if (test)
  message("--test on")
  enable_testing()
  ADD_SUBDIRECTORY(test_cases)
  ADD_SUBDIRECTORY(tests)
  ADD_SUBDIRECTORY(examples)
endif()

# use GNU install dirs (e.g. lib64 instead of lib)
INCLUDE(GNUInstallDirs)
  
  
