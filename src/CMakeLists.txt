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

# WAGASCI include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# ROOT include directory
include_directories(${ROOT_INCLUDE_DIRS})

# List all the source files in this directory
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

# Compile them as a static library and dynamic library
add_library(libwagasci SHARED ${SOURCES})

# Set the library name (dont use the wrong "liblibwagasci" name)
set_target_properties(libwagasci PROPERTIES OUTPUT_NAME "wagasci")

# Link with ROOT and boost libraries
if ( APPLE )
  target_link_libraries(libwagasci ${ROOT_LIBRARIES} Boost::system Boost::filesystem)
else()
  target_link_libraries(libwagasci ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
endif()

# make the library discoverable from all the programs
target_include_directories(libwagasci PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# install the static library in the lib folder
install(TARGETS libwagasci DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
