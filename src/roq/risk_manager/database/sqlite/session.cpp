/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite/session.hpp"

#include "roq/risk_manager/database/sqlite/trades.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

// === HELPERS ===

namespace {
auto create_connection(auto &params) {
  return std::make_unique<third_party::sqlite::Connection>(params);
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params) : connection_{create_connection(params)} {
  Trades::create(*connection_);
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
  Trades::select(*connection_, callback);
}

void Session::operator()(std::span<Trade const> const &trades) {
  Trades::insert(*connection_, trades);
}

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
