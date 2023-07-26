set(TARGET_NAME ${PROJECT_NAME}-database)

set(SOURCES factory.cpp sqlite.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-third_party-sqlite fmt::fmt)
