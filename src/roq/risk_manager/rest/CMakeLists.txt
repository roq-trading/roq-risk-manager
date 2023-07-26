set(TARGET_NAME ${PROJECT_NAME}-rest)

set(SOURCES session.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt)
