set(TARGET_NAME ${PROJECT_NAME}-control)

set(SOURCES session.cpp manager.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE roq-web::roq-web roq-io::roq-io roq-api::roq-api
                                             fmt::fmt)
