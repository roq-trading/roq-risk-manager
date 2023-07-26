/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Session final : public roq::web::rest::Server::Handler {
  struct Disconnected final {
    uint64_t session_id = {};
  };

  struct Handler {
    virtual void operator()(Disconnected const &) = 0;
  };

  Session(Handler &, uint64_t session_id, roq::io::net::tcp::Connection::Factory &);

 protected:
  void close();

  // web::rest::Server::Handler
  void operator()(roq::web::rest::Server::Disconnected const &) override;
  void operator()(roq::web::rest::Server::Request const &) override;
  void operator()(roq::web::rest::Server::Text const &) override;
  void operator()(roq::web::rest::Server::Binary const &) override;

 private:
  Handler &handler_;
  uint64_t const session_id_;
  std::unique_ptr<roq::web::rest::Server> server_;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
