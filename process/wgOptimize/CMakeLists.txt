set(process wgOptimize)

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
#                            Library                                #
#                                                                   #
#####################################################################

# Compile them as a static library .a
add_library(lib${process} SHARED lib${process}.cpp)
set_target_properties(lib${process} PROPERTIES OUTPUT_NAME "${process}")

# Link with ...
target_link_libraries(lib${process}
 ${ROOT_LIBRARIES}            # ROOT libraries
 ${Boost_FILESYSTEM_LIBRARY}  # boost filesystem libraries
 ${Boost_SYSTEM_LIBRARY}      # boost system libraries
 libwagasci                   # WAGASCI dynamic library
 )

# make the library discoverable from all the programs
target_include_directories(lib${process} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# install the static library in the lib folder
install(TARGETS lib${process} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

#####################################################################
#                                                                   #
#                          Executable                               #
#                                                                   #
#####################################################################

add_executable(${process} ${process}.cpp)

# Link with ...
target_link_libraries(${process} lib${process})

# install the executable in the bin folder
install(TARGETS ${process} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
