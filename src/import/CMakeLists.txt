
set (INCLUDES
  ${CMAKE_SOURCE_DIR}
)

include_directories (${INCLUDES})

set (SOURCES
  main.cpp
  ${CMAKE_SOURCE_DIR}/Resource/Variant.cpp
)

add_executable (import ${SOURCES})
target_link_libraries (import resource datastore)

get_target_property (exe_location import LOCATION)
add_custom_command (TARGET import POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${exe_location}" "${EXAMPLE_DIR}")
