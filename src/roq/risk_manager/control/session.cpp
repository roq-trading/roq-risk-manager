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
        account.create_time_utc_min.count(),
        account.create_time_utc_max.count(),
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
        position.create_time_utc.count());
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
    fmt::format_to(
        std::back_inserter(result),
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
        trade.create_time_utc.count(),
        json::String{trade.external_account},
        json::String{trade.external_order_id},
        json::String{trade.external_trade_id});
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
  auto filter = [&](auto &account_2, auto &currency_2) -> bool {
    auto result = false;
    if (!std::empty(account) && account != account_2)
      result = true;
    if (!std::empty(currency) && currency != currency_2)
      result = true;
    log::debug(R"(account="{}", currency="{}", result={})"sv, account_2, currency_2, result);
    return result;
  };
  auto callback = [&](auto &account, auto &currency, auto &funds) {
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
        json::String{account},
        json::String{currency},
        json::Number{funds.balance},  // XXX TODO precision
        json::Number{funds.hold},     // XXX TODO precision
        funds.exchange_time_utc.count(),
        json::String{funds.external_account});
  };
  for (auto &[source, accounts] : shared_2_.accounts_by_source)
    for (auto &[account_2, account_3] : accounts)
      for (auto &[currency_2, funds] : account_3.funds)
        if (!filter(account_2, currency_2))
          callback(account_2, currency_2, funds);
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
      else if (key == "create_time_utc"sv || key == "exchange_time_utc"sv) {
        // XXX TODO
        // correction.create_time_utc = value.template get<std::string_view>();
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
  auto create_time_utc = convert_to_timestamp<std::chrono::nanoseconds>(end_time_as_string);
  auto compress = database::Compress{
      .create_time_utc = create_time_utc,
  };
  database_(compress);
  response(web::http::Status::OK, web::http::ContentType::APPLICATION_JSON, R"({{"success":{}}})"sv, true);
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
