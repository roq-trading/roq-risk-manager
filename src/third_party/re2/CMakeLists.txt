set(TARGET_NAME "${PROJECT_NAME}-third_party-re2")

set(SOURCES regular_expression.cpp)

add_library("${TARGET_NAME}" OBJECT ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-api::roq-api fmt::fmt re2::re2)
