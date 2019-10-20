set(test1 test_decoder_utils)
set(test2 test_raw_data)

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
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../include")

# ROOT include directory
include_directories("${ROOT_INCLUDE_DIRS}")

#####################################################################
#                                                                   #
#                          Executables                              #
#                                                                   #
#####################################################################

##### test_chip_number

add_executable(${test1} ${test1}.cpp)

# Link with ...
target_link_libraries(${test1} libwgDecoder libwgRawEmulator)

# install the executable in the unit_tests folder
install(TARGETS ${test1} DESTINATION "${CMAKE_INSTALL_PREFIX}/unit_tests")

##### test_raw_data

add_executable(${test2} ${test2}.cpp)

# Link with ...
target_link_libraries(${test2} libwgDecoder libwgRawEmulator)

# install the executable in the unit_tests folder
install(TARGETS ${test2} DESTINATION "${CMAKE_INSTALL_PREFIX}/unit_tests")
