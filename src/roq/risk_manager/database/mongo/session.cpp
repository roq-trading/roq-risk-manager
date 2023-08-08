/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/mongo/session.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace mongo {

// === HELPERS ===

namespace {
auto create_connection(auto &params) {
  std::string params_2{params};
  mongocxx::uri uri{params_2};
  return mongocxx::client{uri};
}

auto create_database(auto &connection, auto const &name) {
  std::string name_2{name};
  return mongocxx::database{connection.database(name_2)};
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params)
    : connection_{create_connection(params)}, database_{create_database(connection_, "admin"sv)} {
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

}  // namespace mongo
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
