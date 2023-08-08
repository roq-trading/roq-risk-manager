/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/factory.hpp"

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

#include "roq/risk_manager/database/sqlite/session.hpp"

#if defined(BUILD_CLICKHOUSE)
#include "roq/risk_manager/database/clickhouse/session.hpp"
#endif

#if defined(BUILD_MONGO)
#include "roq/risk_manager/database/mongo/session.hpp"
#endif

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {

std::unique_ptr<Session> Factory::create(std::string_view const &type, std::string_view const &params) {
  if (utils::case_insensitive_compare(type, "sqlite"sv) == 0 ||
      utils::case_insensitive_compare(type, "sqlite3"sv) == 0) {
    return std::make_unique<sqlite::Session>(params);
#if defined(BUILD_CLICKHOUSE)
  } else if (utils::case_insensitive_compare(type, "clickhouse"sv) == 0) {
    return std::make_unique<clickhouse::Session>(params);
#endif
#if defined(BUILD_MONGO)
  } else if (
      utils::case_insensitive_compare(type, "mongo"sv) == 0 ||
      utils::case_insensitive_compare(type, "mongodb"sv) == 0) {
    return std::make_unique<mongo::Session>(params);
#endif
  } else {
    log::fatal(R"(Unexpected: database type="{}")"sv, type);
  }
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
