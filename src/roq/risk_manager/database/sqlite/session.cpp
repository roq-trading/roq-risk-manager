/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite/session.hpp"

#include "roq/risk_manager/database/sqlite/funds.hpp"
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
  Funds::create(*connection_);
}

// query

void Session::operator()(std::function<void(Account const &)> const &callback) {
  Trades::select(*connection_, callback);
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
  Trades::select(*connection_, callback);
}

void Session::operator()(
    std::function<void(Trade const &)> const &callback,
    std::string_view const &account,
    std::chrono::nanoseconds start_time) {
  Trades::select(*connection_, callback, account, start_time);
}

void Session::operator()(
    std::function<void(database::Funds const &)> const &callback,
    std::string_view const &account,
    std::string_view const &currency) {
  Funds::select(*connection_, callback, account, currency);
}

// insert

void Session::operator()(std::span<Trade const> const &trades) {
  Trades::insert(*connection_, trades);
}

void Session::operator()(std::span<Correction const> const &corrections) {
  Trades::insert(*connection_, corrections);
}

void Session::operator()(std::span<database::Funds const> const &funds) {
  Funds::insert(*connection_, funds);
}

// maintenance

void Session::operator()(Compress const &compress) {
  Trades::compress(*connection_, compress.exchange_time_utc);
}

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
