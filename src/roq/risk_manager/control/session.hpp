/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

#include "roq/risk_manager/shared.hpp"

#include "roq/risk_manager/database/session.hpp"

#include "roq/risk_manager/control/response.hpp"
#include "roq/risk_manager/control/shared.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Session final : public web::rest::Server::Handler {
  struct Disconnected final {
    uint64_t session_id = {};
  };

  struct Upgraded final {
    uint64_t session_id = {};
  };

  struct Handler {
    virtual void operator()(Disconnected const &) = 0;
    virtual void operator()(Upgraded const &) = 0;
  };

  Session(
      Handler &,
      uint64_t session_id,
      io::net::tcp::Connection::Factory &,
      Shared &,
      risk_manager::Shared const &,
      database::Session &);

  void operator()(database::Trade const &);

 protected:
  bool ready() const;
  bool zombie() const;

  void close();

  // web::rest::Server::Handler
  void operator()(web::rest::Server::Disconnected const &) override;
  void operator()(web::rest::Server::Request const &) override;
  void operator()(web::rest::Server::Text const &) override;
  void operator()(web::rest::Server::Binary const &) override;

  // rest

  void route(Response &, web::rest::Server::Request const &, std::span<std::string_view> const &path);

  void get_accounts(Response &, web::rest::Server::Request const &);
  void get_positions(Response &, web::rest::Server::Request const &);
  void get_trades(Response &, web::rest::Server::Request const &);
  void get_funds(Response &, web::rest::Server::Request const &);

  void put_trade(Response &, web::rest::Server::Request const &);
  void put_compress(Response &, web::rest::Server::Request const &);

  // ws

  void process(std::string_view const &message);

  void process_jsonrpc(std::string_view const &method, auto const &params, auto const &id);

  // helpers

  void send_result(std::string_view const &message, auto const &id);
  void send_error(std::string_view const &message, auto const &id);

  void send_jsonrpc(std::string_view const &type, std::string_view const &message, auto const &id);

  template <typename... Args>
  void send_text(fmt::format_string<Args...> const &, Args &&...);

 private:
  Handler &handler_;
  uint64_t const session_id_;
  std::unique_ptr<web::rest::Server> server_;
  Shared &shared_;
  enum class State { WAITING, READY, ZOMBIE } state_ = {};
  risk_manager::Shared const &shared_2_;
  database::Session &database_;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
