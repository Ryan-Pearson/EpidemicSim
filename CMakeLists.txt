#=============
# CMake setup
#=============

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Enable policies if applicable
foreach (pol
  CMP0025 # Using AppleClang instead of clang on mac
  CMP0042 # Enable @rpaths in target's install name
  CMP0054 # Only interpret if arguments as variables or keywords when unquoted
  CMP0063 # Honor visibility properties for all target types
)
  if (POLICY ${pol})
    cmake_policy(SET ${pol} NEW)
  endif()
endforeach()

# disable this for now, need to sort out how we do versioning....
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 OLD)
endif()

# Declare the project
project(epidemic_sim CXX)

# Checking the install prefix and setting it if none was provided
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  get_filename_component(DIR_ONE_ABOVE ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
  SET(CMAKE_INSTALL_PREFIX ${DIR_ONE_ABOVE}/install CACHE PATH "Setting the install prefix to the default location" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Some modules will generate export headers
include(GenerateExportHeader)

# use folders in IDE projects, this requires that the FOLDER property of targets be set
set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# Defaulting to debug mode
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
       FORCE)
endif(NOT CMAKE_BUILD_TYPE)

#=====================
# Configure the toolchain
#=====================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#=====================
# Enable unit tests, if specified
#=====================
if (BUILD_UNIT_TESTS)
    enable_testing()
    set(APL_TEST_LOG_DIR ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary/LastTestLogs)
endif(BUILD_UNIT_TESTS)

################################
# Adding modules
################################
add_subdirectory(modules)
add_subdirectory(apps)
