/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/session.hpp"

#include <nlohmann/json.hpp>

#include <charconv>
#include <chrono>

#include "roq/exceptions.hpp"

#include "roq/logging.hpp"

#include "roq/web/rest/server_factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace control {

// === HELPERS ===

namespace {
template <typename R>
R convert_to_timestamp(auto &value) {
  using result_type = std::remove_cvref<R>::type;
  if (std::empty(value))
    return {};
  int64_t result = {};
  auto [_, error_code] = std::from_chars(std::data(value), std::data(value) + std::size(value), result);
  if (error_code == std::errc{}) {
    // XXX TODO: check range
    return result_type{result};
  }
  switch (error_code) {
    using namespace std::literals;
    case std::errc::invalid_argument:
      throw InvalidArgument{R"(parse: "{}")"sv, value};
    case std::errc::result_out_of_range:
      throw RangeError{R"(parse: "{}")"sv, value};
    default:
      throw RuntimeError{R"(parse: "{}")"sv, value};
  }
  return {};
}
}  // namespace

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
      if (path[0] == "accounts"sv) {
        if (std::size(path) == 1)
          get_accounts(response, request);
      } else if (path[0] == "trades_by_account"sv) {
        if (std::size(path) == 1)
          get_trades_by_account(response, request);
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

void Session::get_accounts(Response &response, roq::web::rest::Server::Request const &request) {
  if (!std::empty(request.query))
    throw RuntimeError{"Unexpected: query keys not supported"sv};
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

void Session::get_trades_by_account(Response &response, roq::web::rest::Server::Request const &request) {
  std::string_view account, start_time;
  for (auto &[key, value] : request.query) {
    if (key == "account"sv)
      account = value;
    else if (key == "start_time"sv)
      start_time = value;
    else
      throw RuntimeError{R"(Unexpected: query key="{}" not supported)"sv, key};
  }
  auto tmp = convert_to_timestamp<std::chrono::nanoseconds>(start_time);
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
