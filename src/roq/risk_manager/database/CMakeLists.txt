set(TARGET_NAME ${PROJECT_NAME}-database)

add_subdirectory(sqlite)

set(SOURCES factory.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-database-sqlite fmt::fmt)

install(TARGETS ${TARGET_NAME})
