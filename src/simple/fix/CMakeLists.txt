set(TARGET_NAME "${PROJECT_NAME}-fix")

set(SOURCES session.cpp)

add_library("${TARGET_NAME}" OBJECT ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-api::roq-api fmt::fmt)
