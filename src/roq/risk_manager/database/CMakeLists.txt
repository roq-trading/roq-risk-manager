set(TARGET_NAME ${PROJECT_NAME}-database)

add_subdirectory(sqlite)

set(SOURCES factory.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PROJECT_NAME}-database-sqlite ${PROJECT_NAME}-third_party-sqlite
                         roq-logging::roq-logging fmt::fmt)

if(APPLE)
  target_compile_definitions(${TARGET_NAME} PRIVATE FMT_USE_NONTYPE_TEMPLATE_ARGS=1)
endif()

install(TARGETS ${TARGET_NAME})
