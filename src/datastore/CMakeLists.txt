
set (INCLUDES
  ${CMAKE_SOURCE_DIR}
)

include_directories (${INCLUDES})

set (SOURCES
  Database.cpp
  FieldDescriptor.cpp
  FieldType.cpp
  JsonStorage.cpp
  Logic.cpp
)

add_library(datastore ${SOURCES})

