/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/rest/session.hpp"

#include "roq/web/rest/server_factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace rest {

// === IMPLEMENTATION ===

Session::Session(Handler &handler, uint64_t session_id, roq::io::net::tcp::Connection::Factory &factory)
    : handler_{handler}, session_id_{session_id}, server_{roq::web::rest::ServerFactory::create(*this, factory)} {
}

void Session::close() {
  (*server_).close();
}

// web::rest::Server::Handler

void Session::operator()(roq::web::rest::Server::Disconnected const &) {
}

void Session::operator()(roq::web::rest::Server::Request const &) {
}

void Session::operator()(roq::web::rest::Server::Text const &) {
}

void Session::operator()(roq::web::rest::Server::Binary const &) {
}

}  // namespace rest
}  // namespace risk_manager
}  // namespace roq
