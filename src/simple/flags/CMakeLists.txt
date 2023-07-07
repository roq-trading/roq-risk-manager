set(TARGET_NAME "${PROJECT_NAME}-simple-flags")

set(SOURCES flags.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS flags.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "simple/flags"
  OUTPUT_TYPE
  "flags"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(flags-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "simple/flags"
  OUTPUT_TYPE
  "flags"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

target_link_libraries("${TARGET_NAME}" absl::flags)
