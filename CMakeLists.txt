cmake_minimum_required(VERSION 3.0)
project(dart LANGUAGES CXX)

##############
# basic definitions
##############
# compilation parameters
add_compile_options(-std=c++17 -Wno-unused-result)

# set if the exec must be configured for debug or release/optimized
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include_directories(${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/without_gui/include/
)

##############
# check dependencies
##############
if(NOT DEFINED ENV{GUROBI_HOME})
    message(FATAL_ERROR "You must set GUROBI_HOME environment variable")
endif()

# add additional cmake definitions
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/cmake/")

# Gurobi add_dependency. It must be version 8.1 !
# It will find Gurobi based on GUROBI_HOME
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})

# YAML parser dependency
find_package(YAML-CPP REQUIRED
    HINTS /usr/local/share/cmake)
include_directories(${YAML_CPP_INCLUDE_DIRS})

##############
# configurable parameters
##############
option(PARTITIONING_MODE "Enables partitioning mode" OFF)
message(STATUS "Partitioning mode: '${PARTITIONING_MODE}'")

set(FPGA "pynq" CACHE STRING
  "FPGA board chosen for DART")
set(FPGAValues "pynq;zcu_102;us_96" CACHE STRING
  "List of possible FPGAs supported by DART")
set_property(CACHE FPGA PROPERTY STRINGS ${FPGAValues})
message(STATUS "Supported FPGA: '${FPGA}'")

##############
# determining the source files to be compiled based on the configurable parameters
##############
set (SRC_FILES
    without_gui/src/main.cpp
)
if (NOT ${PARTITIONING_MODE})
    add_compile_options(-DWITHOUT_PARTITIONING)
    list(APPEND SRC_FILES
        without_gui/src/pr_tool.cpp
        without_gui/src/flora.cpp
    )
else()
    add_compile_options(-DWITH_PARTITIONING)
    list(APPEND SRC_FILES
        without_gui/src/pr_tool.cpp
        src/partition.cpp
        without_gui/src/flora.cpp
    )
endif()

##############
# set source and definitions based on the fpga model
##############
add_subdirectory("src/devices/${FPGA}")
# DEVICE_DEFINE is created in the child scope and copied back to the parent scope
add_definitions(${DEVICE_DEFINE})

##############
# get git revision and date running these two commands:
# - git log -1 --format=%cd 
# - git rev-parse --short HEAD
##############
IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(GIT_FOUND)
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE "GIT_BUILD_VERSION"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE( STATUS "Git version: ${GIT_BUILD_VERSION}" )
    add_compile_options(-DGIT_REV="${GIT_BUILD_VERSION}")
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd 
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE "GIT_DATE"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE( STATUS "Git commit date: ${GIT_DATE}" )
    add_compile_options(-DGIT_DATE="${GIT_DATE}")
    # TODO: ideally these definitions would only apply to the main.cpp, avoiding recompiling all the code base every commit. However, the command below is not working
    #add_compile_definitions(${CMAKE_CURRENT_SOURCE_DIR}/without_gui/src/main.cpp -DGIT_DATE=\\"${GIT_DATE}\\" -DGIT_REV=\\"${GIT_BUILD_VERSION}\\" )
  ELSE(GIT_FOUND)
    SET(GIT_BUILD_VERSION 0)
    SET(GIT_DATE 0)
  ENDIF(GIT_FOUND)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

##############
# set libs
##############
target_link_libraries(${CMAKE_PROJECT_NAME} m stdc++fs ${GUROBI_LIBRARY} gurobi_g++5.2 gurobi81 yaml-cpp)
