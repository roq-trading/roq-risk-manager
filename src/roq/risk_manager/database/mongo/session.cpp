/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/mongo/session.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace mongo {

// === HELPERS ===

namespace {}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params) {
}

// query

void Session::operator()(std::function<void(Account const &)> const &callback) {
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
}

void Session::operator()(
    std::function<void(Trade const &)> const &callback,
    std::string_view const &account,
    std::chrono::nanoseconds start_time) {
}

// insert

void Session::operator()(std::span<Trade const> const &trades) {
}

}  // namespace mongo
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
