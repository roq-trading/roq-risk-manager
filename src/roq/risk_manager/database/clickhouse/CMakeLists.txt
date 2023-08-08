set(TARGET_NAME ${PROJECT_NAME}-database-clickhouse)

set(SOURCES session.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt ${CLICKHOUSE_CPP_LIBRARIES}
                                             ${CITYHASH_LIBRARIES} ${LZ4_LIBRARIES})
