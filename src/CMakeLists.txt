cmake_minimum_required(VERSION 2.8)

project (query-challenge)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif ()

add_subdirectory(datastore)
add_subdirectory(Resource)
add_subdirectory(query)
add_subdirectory(import)
