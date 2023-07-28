set(TARGET_NAME ${PROJECT_NAME}-database-sqlite)

set(SOURCES session.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-third_party-sqlite fmt::fmt)
