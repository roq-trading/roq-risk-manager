set(TARGET_NAME ${PROJECT_NAME}-database-sqlite)

set(SOURCES funds.cpp session.cpp trades.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

if(APPLE)
  target_compile_definitions(${TARGET_NAME} PRIVATE FMT_USE_NONTYPE_TEMPLATE_ARGS=1)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-third_party-sqlite fmt::fmt)
