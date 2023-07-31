/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

#include "roq/risk_manager/control/response.hpp"
#include "roq/risk_manager/control/shared.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Session final : public web::rest::Server::Handler {
  struct Disconnected final {
    uint64_t session_id = {};
  };

  struct Handler {
    virtual void operator()(Disconnected const &) = 0;
  };

  Session(Handler &, uint64_t session_id, io::net::tcp::Connection::Factory &, Shared &);

 protected:
  void close();

  // web::rest::Server::Handler
  void operator()(web::rest::Server::Disconnected const &) override;
  void operator()(web::rest::Server::Request const &) override;
  void operator()(web::rest::Server::Text const &) override;
  void operator()(web::rest::Server::Binary const &) override;

  void route(Response &, web::rest::Server::Request const &, std::span<std::string_view> const &path);

  void get_positions(Response &, web::rest::Server::Request const &);

 private:
  Handler &handler_;
  uint64_t const session_id_;
  std::unique_ptr<web::rest::Server> server_;
  Shared shared_;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
