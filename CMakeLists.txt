cmake_minimum_required(VERSION 3.12)

# cmake

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake
                      ${CMAKE_MODULE_PATH})

# project

project(roq-risk-manager)

# conda

if(DEFINED ENV{CONDA_PREFIX} AND NOT DEFINED ENV{CONDA_BUILD})
  set(CMAKE_INSTALL_PREFIX $ENV{CONDA_PREFIX})
  set(CMAKE_FIND_ROOT_PATH $ENV{CONDA_PREFIX})
endif()

# rpath

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# c++

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

# filesystem

include(GNUInstallDirs)

# dependencies

find_package(absl REQUIRED)
find_package(fmt REQUIRED)
find_package(roq-api REQUIRED)
find_package(roq-client REQUIRED)
find_package(roq-flags REQUIRED)
find_package(roq-logging REQUIRED)
find_package(roq-io REQUIRED)
find_package(roq-web REQUIRED)

option(BUILD_CLICKHOUSE "Support ClickHouse" OFF)

if(BUILD_CLICKHOUSE)
  find_package(ClickHouse_CPP REQUIRED)
  find_library(LZ4_LIBRARIES lz4 REQUIRED)
  find_library(CITYHASH_LIBRARIES cityhash REQUIRED)
endif()

option(BUILD_MONGO "Support MongoDB" OFF)

if(BUILD_MONGO)
  find_package(Boost REQUIRED)
  find_package(mongocxx REQUIRED)
endif()

if(UNIX AND NOT APPLE)
  set(RT_LIBRARIES rt)
else()
  set(RT_LIBRARIES)
endif()

find_package(PkgConfig REQUIRED)
find_package(Catch2 REQUIRED)

find_program(CLANG_FORMAT clang-format REQUIRED)

pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# include

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  # >>> autogen >>>
  ${CMAKE_BINARY_DIR}/src)

# sub-projects

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

# test

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/test)

# benchmark

find_package(benchmark REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark)

# config

install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

install(FILES ${CMAKE_SOURCE_DIR}/CHANGELOG.md
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME})
