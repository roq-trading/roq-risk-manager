set(TARGET_NAME ${PROJECT_NAME}-database)

add_subdirectory(sqlite)

set(SOURCES factory.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCES})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PROJECT_NAME}-database-sqlite ${PROJECT_NAME}-third_party-sqlite
                         roq-logging::roq-logging fmt::fmt)

if(APPLE)
  target_compile_definitions(${TARGET_NAME} PRIVATE FMT_USE_NONTYPE_TEMPLATE_ARGS=1)
endif()

install(TARGETS ${TARGET_NAME})

if(BUILD_CLICKHOUSE)
  add_subdirectory(clickhouse)
  target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_CLICKHOUSE=1)
  target_link_libraries(
    ${TARGET_NAME} PRIVATE ${PROJECT_NAME}-database-clickhouse ${CLICKHOUSE_CPP_LIBRARIES}
                           ${CITYHASH_LIBRARIES} ${LZ4_LIBRARIES})
endif()

if(BUILD_MONGO)
  add_subdirectory(mongo)
  target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_MONGO=1)
  target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-database-mongo
                                               mongo::mongocxx_shared)
endif()
