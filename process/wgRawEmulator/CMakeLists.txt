set(raw_emulator wgRawEmulator)
set(decoder wgDecoder)

################ Compiler flags ################

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

################ Linker flags ################

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

################ includes ################

# WAGASCI include directory
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# ROOT include directory
include_directories("${ROOT_INCLUDE_DIRS}")

#####################################################################
#                                                                   #
#                              Library                              #
#                                                                   #
#####################################################################

##### wgRawEmulator

add_library(lib${raw_emulator} SHARED lib${raw_emulator}.cpp)
set_target_properties(lib${raw_emulator} PROPERTIES OUTPUT_NAME "${raw_emulator}")

# make the library discoverable from all the programs
target_include_directories(lib${raw_emulator} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# install the static library in the lib folder
install(TARGETS lib${raw_emulator} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")


#####################################################################
#                                                                   #
#                          Executables                              #
#                                                                   #
#####################################################################

##### wgRawEmulator

add_executable(${raw_emulator} ${raw_emulator}.cpp)

# Link with ...
target_link_libraries(${raw_emulator} lib${decoder} lib${raw_emulator})

# install the executable in the bin folder
install(TARGETS ${raw_emulator} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

##### Unit tests

add_subdirectory(unit_tests)
