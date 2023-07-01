set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES response.cpp session.cpp)

add_library("${TARGET_NAME}" OBJECT ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-api::roq-api fmt::fmt)
