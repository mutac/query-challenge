
set (INCLUDES
  ${CMAKE_SOURCE_DIR}
)

include_directories (${INCLUDES})

set (SOURCES
  main.cpp
  ${CMAKE_SOURCE_DIR}/Resource/Variant.cpp
)

add_executable(query ${SOURCES})
target_link_libraries(query resource datastore)

get_target_property (exe_location query LOCATION)
add_custom_command (TARGET query POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${exe_location}" "${EXAMPLE_DIR}")
