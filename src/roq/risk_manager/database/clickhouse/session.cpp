/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/clickhouse/session.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace clickhouse {

// === IMPLEMENTATION ===

Session::Session([[maybe_unused]] std::string_view const &params) {
}

// query

void Session::operator()(std::function<void(Account const &)> const &callback) {
  // XXX TODO
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
  // XXX TODO
}

void Session::operator()(
    std::function<void(Trade const &)> const &callback,
    std::string_view const &account,
    std::chrono::nanoseconds start_time) {
  // XXX TODO
}

// insert

void Session::operator()(std::span<Trade const> const &trades) {
  // XXX TODO
}

void Session::operator()(std::span<Correction const> const &corrections) {
  // XXX TODO
}

void Session::operator()(Compress const &compress) {
  // XXX TODO
}

}  // namespace clickhouse
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
