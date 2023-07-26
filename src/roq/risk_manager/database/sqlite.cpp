/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {

// === HELPERS ===

namespace {
auto create_connection(auto &params) {
  return std::make_unique<third_party::sqlite::Connection>(params);
}
}  // namespace

// === IMPLEMENTATION ===

SQLite::SQLite(std::string_view const &params) : connection_{create_connection(params)} {
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
