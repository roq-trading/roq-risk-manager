/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/clickhouse/session.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace clickhouse {

// === HELPERS ===

namespace {
auto create_client(auto &params) {
  ::clickhouse::ClientOptions options;
  // XXX TODO better params parsing
  std::string params_2{params};
  options.SetHost(params_2);
  // options.SetPort(settings.db_port);
  // options.SetSendRetries(3);
  // options.SetRetryTimeout(5s);
  // options.TcpKeepAlive(true);
  return ::clickhouse::Client{options};
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params) : client_{create_client(params)} {
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

void Session::operator()(
    std::function<void(Funds const &)> const &callback,
    std::string_view const &account,
    std::string_view const &currency) {
  // XXX TODO
}

// insert

void Session::operator()(std::span<Trade const> const &trades) {
  // XXX TODO
}

void Session::operator()(std::span<Correction const> const &corrections) {
  // XXX TODO
}

void Session::operator()(std::span<Funds const> const &funds) {
  // XXX TODO
}

// maintenance

void Session::operator()(Compress const &compress) {
  // XXX TODO
}

}  // namespace clickhouse
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
