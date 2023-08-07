set(TARGET_NAME ${PROJECT_NAME}-database-mongo)

set(SOURCES session.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE mongo::mongocxx_shared fmt::fmt)
