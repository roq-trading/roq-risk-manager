set(TARGET_NAME "${PROJECT_NAME}-benchmark")

set(SOURCES main.cpp)

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" PRIVATE roq-logging::roq-logging benchmark::benchmark
                                               ${RT_LIBRARIES})

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

install(TARGETS ${TARGET_NAME})
