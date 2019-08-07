if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# CMAKE 3.10.2 is the default version in the Ubuntu 18.04 repositories
cmake_minimum_required (VERSION 3.10.2)

# Name of the project
project(Analysis)

if ( NOT UNIX )
  message( FATAL_ERROR "Non UNIX OSes are not supported" )
endif()

# Set C++11 standard
set (CMAKE_CXX_STANDARD 11)

# Enable debug symbols
set(CMAKE_BUILD_TYPE Debug)

# Be verbose
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# boost system and filesystem are required
find_package(Boost COMPONENTS system filesystem REQUIRED)

# ROOT
find_package(ROOT CONFIG REQUIRED)
include(${ROOT_USE_FILE})
execute_process (
  COMMAND bash -c "root-config --features | grep minuit2 | wc -l"
  OUTPUT_VARIABLE ROOT_HAS_MINUIT2
  )
string(STRIP ${ROOT_HAS_MINUIT2} ROOT_HAS_MINUIT2)
if (ROOT_HAS_MINUIT2 EQUAL "0")
  add_definitions(-DROOT_HAS_NOT_MINUIT2)
  message(AUTHOR_WARNING "ROOT has no support for Minuit2 minimizer")
else ()
  message(STATUS "ROOT supports Minuit2 minimizer")
endif ()

# nlohmann_json JSON parser
if ( APPLE )
  set(nlohmann_json_DIR "/opt/local/lib/cmake/nlohmann_json")
else ()
  set(nlohmann_json_DIR "/usr/lib/cmake")
endif()
find_package(nlohmann_json REQUIRED)

# build libwagasci.a
add_subdirectory(src)

# build all the programs in process folder
add_subdirectory(process)
