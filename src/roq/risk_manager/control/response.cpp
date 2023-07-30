/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/response.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace control {

// === CONSTANTS ===

namespace {
auto const CACHE_CONTROL_NO_STORE = "no-store"sv;
}  // namespace

// === IMPLEMENTATION ===

Response::Response(web::rest::Server &server, web::rest::Server::Request const &request, std::string &encode_buffer)
    : server_{server}, request_{request}, encode_buffer_{encode_buffer} {
}

void Response::send(web::http::Status status, web::http::ContentType content_type, std::string_view const &body) {
  auto connection = [&]() {
    if (status != web::http::Status::OK)  // XXX maybe only close based on category ???
      return web::http::Connection::CLOSE;
    return request_.headers.connection;
  }();
  auto response = web::rest::Server::Response{
      .status = status,
      .connection = connection,
      .sec_websocket_accept = {},
      .cache_control = CACHE_CONTROL_NO_STORE,
      .content_type = content_type,
      .body = body,
  };
  server_.send(response);
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
