/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/session.hpp"

#include <nlohmann/json.hpp>

#include <charconv>
#include <chrono>

#include "roq/logging.hpp"

#include "roq/exceptions.hpp"

#include "roq/json/datetime.hpp"
#include "roq/json/number.hpp"
#include "roq/json/string.hpp"

#include "roq/web/rest/server_factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace control {

// === CONSTANTS ===

namespace {
auto const JSONRPC_VERSION = "2.0"sv;

auto const UNKNOWN_METHOD = "UNKNOWN_METHOD"sv;
}  // namespace

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

template <typename Context>
void trade_to_json(Context context, database::Trade const &trade) {
  fmt::format_to(
      context,
      R"({{)"
      R"("user":{},)"
      R"("strategy_id":{},)"
      R"("account":{},)"
      R"("exchange":{},)"
      R"("symbol":{},)"
      R"("side":{},)"
      R"("quantity":{},)"
      R"("price":{},)"
      R"("exchange_time_utc":{},)"
      R"("external_account":{},)"
      R"("external_order_id":{},)"
      R"("external_trade_id":{})"
      R"(}})"sv,
      json::String{trade.user},
      trade.strategy_id,
      json::String{trade.account},
      json::String{trade.exchange},
      json::String{trade.symbol},
      json::String{trade.side},
      json::Number{trade.quantity},  // XXX TODO precision
      json::Number{trade.price},     // XXX TODO precision
      trade.exchange_time_utc.count(),
      json::String{trade.external_account},
      json::String{trade.external_order_id},
      json::String{trade.external_trade_id});
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(
    Handler &handler,
    uint64_t session_id,
    io::net::tcp::Connection::Factory &factory,
    Shared &shared,
    risk_manager::Shared const &shared_2,
    database::Session &database)
    : handler_{handler}, session_id_{session_id}, server_{web::rest::ServerFactory::create(*this, factory)},
      shared_{shared}, shared_2_{shared_2}, database_{database} {
}

void Session::operator()(database::Trade const &trade) {
  assert(ready());
  // XXX TODO clean up
  std::string message;
  trade_to_json(std::back_inserter(message), trade);
  send_text("{}"sv, message);
}

bool Session::ready() const {
  return state_ == State::READY;
}

bool Session::zombie() const {
  return state_ == State::ZOMBIE;
}

void Session::close() {
  (*server_).close();
}

// web::rest::Server::Handler

void Session::operator()(web::rest::Server::Disconnected const &) {
  state_ = State::ZOMBIE;
  auto disconnected = Disconnected{
      .session_id = session_id_,
  };
  handler_(disconnected);
}

void Session::operator()(web::rest::Server::Request const &request) {
  log::info("DEBUG request={}"sv, request);
  auto success = false;
  try {
    if (request.headers.connection == roq::web::http::Connection::UPGRADE) {
      roq::log::info("Upgrading session_id={} to websocket..."sv, session_id_);
      (*server_).upgrade(request);
      state_ = State::READY;
      auto upgraded = Upgraded{
          .session_id = session_id_,
      };
      handler_(upgraded);
    } else {
      auto path = request.path;  // note! url path has already been split
      if (!std::empty(path) && !std::empty(shared_.url_prefix) && path[0] == shared_.url_prefix)
        path = path.subspan(1);  // drop prefix
      if (!std::empty(path)) {
        Response response{*server_, request, shared_.encode_buffer};
        route(response, request, path);
      }
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

void Session::operator()(web::rest::Server::Text const &text) {
  roq::log::info(R"(message="{})"sv, text.payload);
  auto success = false;
  try {
    process(text.payload);
    success = true;
  } catch (roq::RuntimeError &e) {
    roq::log::error("Error: {}"sv, e);
  } catch (std::exception &e) {
    roq::log::error("Error: {}"sv, e.what());
  }
  if (!success)
    close();
}

void Session::operator()(web::rest::Server::Binary const &) {
}

void Session::route(
    Response &response, web::rest::Server::Request const &request, std::span<std::string_view> const &path) {
  switch (request.method) {
    using enum web::http::Method;
    case GET:
      if (path[0] == "accounts"sv) {
        if (std::size(path) == 1)
          get_accounts(response, request);
      } else if (path[0] == "positions"sv) {
        if (std::size(path) == 1)
          get_positions(response, request);
      } else if (path[0] == "trades"sv) {
        if (std::size(path) == 1)
          get_trades(response, request);
      } else if (path[0] == "funds"sv) {
        if (std::size(path) == 1)
          get_funds(response, request);
      }
      break;
    case HEAD:
      break;
    case POST:
      break;
    case PUT:
      if (path[0] == "trade"sv) {
        if (std::size(path) == 1)
          put_trade(response, request);
      } else if (path[0] == "compress"sv) {
        if (std::size(path) == 1)
          put_compress(response, request);
      }
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

// get

void Session::get_accounts(Response &response, web::rest::Server::Request const &request) {
  if (!std::empty(request.query))
    throw RuntimeError{"Unexpected: query keys not supported"sv};
  std::string result;  // XXX TODO shared encode buffer
  auto callback = [&](database::Account const &account) {
    if (!std::empty(result))
      fmt::format_to(std::back_inserter(result), ","sv);
    fmt::format_to(
        std::back_inserter(result),
        R"({{)"
        R"("name":{},)"
        R"("exchange_time_utc_min":{},)"
        R"("exchange_time_utc_max":{},)"
        R"("trade_count":{})"
        R"(}})"sv,
        json::String{account.name},
        account.exchange_time_utc_min.count(),
        account.exchange_time_utc_max.count(),
        account.trade_count);
  };
  database_(callback);
  if (std::empty(result)) {
    response(web::http::Status::NOT_FOUND, web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, "[{}]"sv, result);
  }
}

void Session::get_positions(Response &response, web::rest::Server::Request const &request) {
  if (!std::empty(request.query))
    throw RuntimeError{"Unexpected: query keys not supported"sv};
  std::string result;  // XXX TODO shared encode buffer
  auto callback = [&](database::Position const &position) {
    if (!std::empty(result))
      fmt::format_to(std::back_inserter(result), ","sv);
    fmt::format_to(
        std::back_inserter(result),
        R"({{)"
        R"("user":{},)"
        R"("strategy_id":{},)"
        R"("account":{},)"
        R"("exchange":{},)"
        R"("symbol":{},)"
        R"("long_quantity":{},)"
        R"("short_quantity":{},)"
        R"("exchange_time_utc":{})"
        R"(}})"sv,
        json::String{position.user},
        position.strategy_id,
        json::String{position.account},
        json::String{position.exchange},
        json::String{position.symbol},
        json::Number{position.long_quantity},   // XXX TODO precision
        json::Number{position.short_quantity},  // XXX TODO precision
        position.exchange_time_utc.count());
  };
  database_(callback);
  if (std::empty(result)) {
    response(web::http::Status::NOT_FOUND, web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, "[{}]"sv, result);
  }
}

void Session::get_trades(Response &response, web::rest::Server::Request const &request) {
  std::string_view account, start_time_as_string;
  for (auto &[key, value] : request.query) {
    log::debug("key={}, value={}"sv, key, value);
    if (key == "account"sv)
      account = value;
    else if (key == "start_time"sv)
      start_time_as_string = value;
    else
      throw RuntimeError{R"(Unexpected: query key="{}" not supported)"sv, key};
  }
  auto start_time = convert_to_timestamp<std::chrono::nanoseconds>(start_time_as_string);
  std::string result;  // XXX TODO shared encode buffer
  auto callback = [&](database::Trade const &trade) {
    if (!std::empty(result))
      fmt::format_to(std::back_inserter(result), ","sv);
    trade_to_json(std::back_inserter(result), trade);
  };
  database_(callback, account, start_time);
  if (std::empty(result)) {
    response(web::http::Status::NOT_FOUND, web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, "[{}]"sv, result);
  }
}

void Session::get_funds(Response &response, web::rest::Server::Request const &request) {
  std::string_view account, currency;
  for (auto &[key, value] : request.query) {
    log::debug("key={}, value={}"sv, key, value);
    if (key == "account"sv)
      account = value;
    else if (key == "currency"sv)
      currency = value;
    else
      throw RuntimeError{R"(Unexpected: query key="{}" not supported)"sv, key};
  }
  std::string result;  // XXX TODO shared encode buffer
  auto callback = [&](database::Funds const &funds) {
    if (!std::empty(result))
      fmt::format_to(std::back_inserter(result), ","sv);
    fmt::format_to(
        std::back_inserter(result),
        R"({{)"
        R"("account":{},)"
        R"("currency":{},)"
        R"("balance":{},)"
        R"("hold":{},)"
        R"("exchange_time_utc":{},)"
        R"("external_account":{})"
        R"(}})"sv,
        json::String{funds.account},
        json::String{funds.currency},
        json::Number{funds.balance},  // XXX TODO precision
        json::Number{funds.hold},     // XXX TODO precision
        funds.exchange_time_utc.count(),
        json::String{funds.external_account});
  };
  database_(callback, account, currency);
  if (std::empty(result)) {
    response(web::http::Status::NOT_FOUND, web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, "[{}]"sv, result);
  }
}

// put

void Session::put_trade(Response &response, web::rest::Server::Request const &request) {
  if (std::empty(request.body)) {
    // XXX TODO what is a proper response?
    response(web::http::Status::NOT_FOUND, web::http::ContentType::APPLICATION_JSON, R"({{"success":{}}})"sv, false);
    return;
  }
  std::vector<database::Correction> corrections;
  auto parse_item = [&](auto &item) {
    database::Correction correction;
    for (auto &[key, value] : item.items()) {
      if (key == "user"sv)
        correction.user = value.template get<std::string_view>();
      else if (key == "strategy_id"sv)
        correction.strategy_id = value.template get<uint32_t>();
      else if (key == "account"sv)
        correction.account = value.template get<std::string_view>();
      else if (key == "exchange"sv)
        correction.exchange = value.template get<std::string_view>();
      else if (key == "symbol"sv)
        correction.symbol = value.template get<std::string_view>();
      else if (key == "side"sv) {
        auto tmp = value.template get<std::string_view>();
        correction.side = magic_enum::enum_cast<Side>(tmp).value();
      } else if (key == "quantity"sv)
        correction.quantity = value.template get<double>();
      else if (key == "price"sv)
        correction.price = value.template get<double>();
      else if (key == "exchange_time_utc"sv || key == "exchange_time_utc"sv) {
        // XXX TODO
        // correction.exchange_time_utc = value.template get<std::string_view>();
      } else if (key == "reason"sv)
        correction.reason = value.template get<std::string_view>();
      else
        throw RuntimeError{R"(Unexpected: json key="{}" not supported)"sv, key};
    }
    // XXX TODO validation? ... or maybe database should validate?
    corrections.emplace_back(std::move(correction));
  };
  auto json = nlohmann::json::parse(request.body);
  if (json.is_array()) {
    for (auto item : json)  // XXX not sure if reference would work here...
      parse_item(item);
  } else {
    parse_item(json);
  }
  database_(corrections);
  response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, R"({{"success":{}}})"sv, true);
}

void Session::put_compress(Response &response, web::rest::Server::Request const &request) {
  std::string_view end_time_as_string;
  for (auto &[key, value] : request.query) {
    log::debug("key={}, value={}"sv, key, value);
    if (key == "end_time"sv)
      end_time_as_string = value;
    else
      throw RuntimeError{R"(Unexpected: query key="{}" not supported)"sv, key};
  }
  if (!std::empty(request.body)) {
    if (!std::empty(end_time_as_string))
      throw RuntimeError{R"(Unexpected: query params and request body?)"sv};
    auto json = nlohmann::json::parse(request.body);
    for (auto &[key, value] : json.items()) {
      if (key == "end_time"sv)
        end_time_as_string = value.template get<std::string_view>();
      else
        throw RuntimeError{R"(Unexpected: json key="{}" not supported)"sv, key};
    }
  }
  if (std::empty(end_time_as_string))
    throw RuntimeError{R"(Unexpected: no timestamp)"sv};
  auto exchange_time_utc = convert_to_timestamp<std::chrono::nanoseconds>(end_time_as_string);
  auto compress = database::Compress{
      .exchange_time_utc = exchange_time_utc,
  };
  database_(compress);
  response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, R"({{"success":{}}})"sv, true);
}

void Session::process(std::string_view const &message) {
  assert(!zombie());
  auto success = false;
  try {
    auto json = nlohmann::json::parse(message);  // note! not fast... you should consider some other json parser here
    auto version = json.at("jsonrpc"sv).template get<std::string_view>();
    if (version != JSONRPC_VERSION)
      throw roq::RuntimeError{R"(Invalid JSONRPC version ("{}"))"sv, version};
    auto method = json.at("method"sv).template get<std::string_view>();
    auto params = json.at("params"sv);
    auto id = json.at("id"sv);
    process_jsonrpc(method, params, id);
    success = true;
  } catch (roq::RuntimeError &e) {
    roq::log::error("Error: {}"sv, e);
  } catch (std::exception &e) {
    roq::log::error("Error: {}"sv, e.what());
  }
  roq::log::debug("success={}"sv, success);
  if (!success)
    close();
}

void Session::process_jsonrpc(
    [[maybe_unused]] std::string_view const &method, [[maybe_unused]] auto const &params, auto const &id) {
  send_error(UNKNOWN_METHOD, id);  // XXX TODO define protocol
}

void Session::send_result(std::string_view const &message, auto const &id) {
  send_jsonrpc("result"sv, message, id);
}

void Session::send_error(std::string_view const &message, auto const &id) {
  send_jsonrpc("error"sv, message, id);
}

void Session::send_jsonrpc(std::string_view const &type, std::string_view const &message, auto const &id) {
  assert(!zombie());
  // note!
  //   response must echo the id field from the request (same type)
  auto type_2 = id.type();
  switch (type_2) {
    using enum nlohmann::json::value_t;
    case string:
      send_text(
          R"({{)"
          R"("jsonrpc":"{}",)"
          R"("{}":"{}",)"
          R"("id":"{}")"
          R"(}})"sv,
          JSONRPC_VERSION,
          type,
          message,
          id.template get<std::string_view>());
      break;
    case number_integer:
    case number_unsigned:
      send_text(
          R"({{)"
          R"("jsonrpc":"{}",)"
          R"("{}":"{}",)"
          R"("id":{})"
          R"(}})"sv,
          JSONRPC_VERSION,
          type,
          message,
          id.template get<int64_t>());
      break;
    default:
      roq::log::warn("Unexpected: type={}"sv, magic_enum::enum_name(type_2));
  }
}

template <typename... Args>
void Session::send_text(fmt::format_string<Args...> const &fmt, Args &&...args) {
  shared_.encode_buffer.clear();
  fmt::format_to(std::back_inserter(shared_.encode_buffer), fmt, std::forward<Args>(args)...);
  roq::log::debug(R"(message="{}")"sv, shared_.encode_buffer);
  (*server_).send_text(shared_.encode_buffer);
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
