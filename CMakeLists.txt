if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# CMAKE 3.10.2 is the default version in the Ubuntu 18.04 repositories
cmake_minimum_required (VERSION 3.10.2)

# Name of the project
project(Analysis)

# Set C++11 standard
set (CMAKE_CXX_STANDARD 11)

# boost system and filesystem are required
find_package(Boost COMPONENTS system filesystem REQUIRED)

# ROOT
find_package(ROOT CONFIG REQUIRED)

# build libwagasci.a
add_subdirectory(src)

# build all the programs in process folder
add_subdirectory(process)
