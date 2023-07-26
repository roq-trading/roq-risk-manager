set(TARGET_NAME ${PROJECT_NAME}-third_party-sqlite)

set(SOURCES connection.cpp statement.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} fmt::fmt)
