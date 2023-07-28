/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/factory.hpp"

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

#include "roq/risk_manager/database/sqlite/session.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {

std::unique_ptr<Session> Factory::create(std::string_view const &type, std::string_view const &params) {
  if (utils::case_insensitive_compare(type, "sqlite"sv) == 0) {
    return std::make_unique<sqlite::Session>(params);
  } else {
    log::fatal(R"(Unexpected: database type="{}")"sv, type);
  }
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
