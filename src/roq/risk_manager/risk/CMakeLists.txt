set(TARGET_NAME ${PROJECT_NAME}-risk)

set(SOURCES account.cpp instrument.cpp position.cpp strategy.cpp user.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt)
