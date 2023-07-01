/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/json/response.hpp"

using namespace std::literals;

namespace simple {
namespace json {

// === CONSTANTS ===

namespace {
auto const CACHE_CONTROL_NO_STORE = "no-store"sv;
}  // namespace

// === IMPLEMENTATION ===

Response::Response(
    roq::web::rest::Server &server, roq::web::rest::Server::Request const &request, std::string &encode_buffer)
    : server_{server}, request_{request}, encode_buffer_{encode_buffer} {
}

void Response::send(
    roq::web::http::Status status, roq::web::http::ContentType content_type, std::string_view const &body) {
  auto connection = [&]() {
    if (status != roq::web::http::Status::OK)  // XXX maybe only close based on category ???
      return roq::web::http::Connection::CLOSE;
    return request_.headers.connection;
  }();
  auto response = roq::web::rest::Server::Response{
      .status = status,
      .connection = connection,
      .sec_websocket_accept = {},
      .cache_control = CACHE_CONTROL_NO_STORE,
      .content_type = content_type,
      .body = body,
  };
  server_.send(response);
}

}  // namespace json
}  // namespace simple
