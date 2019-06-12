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
#add_library(libwagasci_static STATIC ${SOURCES})
set_target_properties(libwagasci PROPERTIES OUTPUT_NAME "wagasci")
#set_target_properties(libwagasci_static PROPERTIES OUTPUT_NAME "wagasci_static")

# Link with ROOT and boost libraries
target_link_libraries(libwagasci ${ROOT_LIBRARIES}  ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} Boost::system Boost::filesystem)
#target_link_libraries(libwagasci_static
#  ${ROOT_LIBRARIES}
#  ${Boost_FILESYSTEM_LIBRARY}
#  ${Boost_SYSTEM_LIBRARY}
#  )

# make the library discoverable from all the programs
target_include_directories(libwagasci PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_include_directories(libwagasci_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# install the static library in the lib folder
install(TARGETS libwagasci DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
#install(TARGETS libwagasci_static DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
