set(TARGET_NAME ${PROJECT_NAME}-control)

set(SOURCES manager.cpp response.cpp session.cpp shared.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

add_dependencies(${TARGET_NAME} ${PROJECT_NAME}-flags-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-web::roq-web roq-io::roq-io roq-api::roq-api
                                             fmt::fmt)
