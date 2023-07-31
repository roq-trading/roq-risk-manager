/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/session.hpp"

#include <nlohmann/json.hpp>

#include "roq/exceptions.hpp"

#include "roq/logging.hpp"

#include "roq/web/rest/server_factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace control {

// === IMPLEMENTATION ===

Session::Session(Handler &handler, uint64_t session_id, io::net::tcp::Connection::Factory &factory, Shared &shared)
    : handler_{handler}, session_id_{session_id}, server_{web::rest::ServerFactory::create(*this, factory)},
      shared_{shared} {
}

void Session::close() {
  (*server_).close();
}

// web::rest::Server::Handler

void Session::operator()(web::rest::Server::Disconnected const &) {
  auto disconnected = Disconnected{
      .session_id = session_id_,
  };
  handler_(disconnected);
}

void Session::operator()(web::rest::Server::Request const &request) {
  log::info("DEBUG request={}"sv, request);
  auto success = false;
  try {
    auto path = request.path;  // note! url path has already been split
    if (!std::empty(path) && !std::empty(shared_.url_prefix) && path[0] == shared_.url_prefix)
      path = path.subspan(1);  // drop prefix
    if (!std::empty(path)) {
      Response response{*server_, request, shared_.encode_buffer};
      route(response, request, path);
    }
    success = true;
  } catch (RuntimeError &e) {
    log::error("Error: {}"sv, e);
  } catch (std::exception &e) {
    log::error("Error: {}"sv, e.what());
  }
  if (!success)
    close();
}

void Session::operator()(web::rest::Server::Text const &) {
}

void Session::operator()(web::rest::Server::Binary const &) {
}

void Session::route(
    Response &response, roq::web::rest::Server::Request const &request, std::span<std::string_view> const &path) {
  switch (request.method) {
    using enum roq::web::http::Method;
    case GET:
      if (std::size(path) == 1) {
        if (path[0] == "symbols"sv)
          get_positions(response, request);
      }
      break;
    case HEAD:
      break;
    case POST:
      break;
    case PUT:
      break;
    case DELETE:
      break;
    case CONNECT:
      break;
    case OPTIONS:
      break;
    case TRACE:
      break;
  }
}

void Session::get_positions(Response &response, roq::web::rest::Server::Request const &) {
  /*
  if (std::empty(shared_.symbols)) {
    response(roq::web::http::Status::NOT_FOUND, roq::web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(
        roq::web::http::Status::OK,
        roq::web::http::ContentType::APPLICATION_JSON,
        R"(["{}"])"sv,
        fmt::join(shared_.symbols, R"(",")"sv));
  }
  */
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
