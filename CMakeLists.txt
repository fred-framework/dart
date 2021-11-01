cmake_minimum_required(VERSION 3.0)
project(dart LANGUAGES CXX)

if(NOT DEFINED ENV{GUROBI_HOME})
    message(FATAL_ERROR "You must set GUROBI_HOME environment variable")
endif()

# add additional cmake definitions
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/cmake/")

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

# Gurobi add_dependency. It must be version 8.1 !
# It will find Gurobi based on GUROBI_HOME
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})

# YAML parser dependency
find_package(YAML-CPP REQUIRED
    HINTS /usr/local/share/cmake)
include_directories(${YAML_CPP_INCLUDE_DIRS})

set (SRC_FILES without_gui/src/pr_tool.cpp
    without_gui/src/flora.cpp
    src/milp_model_pynq.cpp
    src/csv_data_manipulator.cpp
    without_gui/src/main.cpp
    src/pynq.cpp
    src/pynq_fine_grained.cpp)

add_compile_options(-DFPGA_PYNQ -DWITHOUT_PARTITIONING)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} m stdc++fs ${GUROBI_LIBRARY} gurobi_g++5.2 gurobi81 yaml-cpp)