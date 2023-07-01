/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/json/session.hpp"

#include <nlohmann/json.hpp>

#include <utility>

#include "roq/logging.hpp"

#include "roq/exceptions.hpp"

#include "roq/json/datetime.hpp"
#include "roq/json/number.hpp"

#include "roq/oms/exceptions.hpp"

#include "roq/utils/traits.hpp"

#include "roq/web/rest/server_factory.hpp"

#include "roq/fix/utils.hpp"

#include "simple/error.hpp"

#include "simple/json/business_message_reject.hpp"
#include "simple/json/execution_report.hpp"

using namespace std::literals;

// TODO
// - close handshake doesn't work as per protocol (connection closed too early)

namespace simple {
namespace json {

// === CONSTANTS ===

namespace {
auto const JSONRPC_VERSION = "2.0"sv;

auto const UNKNOWN_METHOD = "UNKNOWN_METHOD"sv;

auto const SUCCESS = "success"sv;
}  // namespace

// === HELPERS ===

namespace {
// map
template <typename T>
T map(std::string_view const &);

template <>
roq::fix::OrdType map(std::string_view const &value) {
  if (value == "MARKET"sv)
    return roq::fix::OrdType::MARKET;
  if (value == "LIMIT"sv)
    return roq::fix::OrdType::LIMIT;
  throw roq::RuntimeError{R"(Unexpected: time_in_force="{}")"sv, value};
}

template <>
roq::fix::Side map(std::string_view const &value) {
  if (value == "BUY"sv)
    return roq::fix::Side::BUY;
  if (value == "SELL"sv)
    return roq::fix::Side::SELL;
  throw roq::RuntimeError{R"(Unexpected: side="{}")"sv, value};
}

template <>
roq::fix::TimeInForce map(std::string_view const &value) {
  if (value == "GTC"sv)
    return roq::fix::TimeInForce::GTC;
  throw roq::RuntimeError{R"(Unexpected: time_in_force="{}")"sv, value};
}

// get
template <typename T>
T get(nlohmann::json::value_type const &value, std::string_view const &key) {
  auto iter = value.find(key);
  if (iter == std::end(value))
    return {};
  if constexpr (std::is_same<T, std::string_view>::value) {
    return (*iter).template get<std::string_view>();
  } else if constexpr (std::is_same<T, roq::utils::Number>::value) {
    auto type = (*iter).type();
    switch (type) {
      using enum nlohmann::json::value_t;
      case string: {
        // note! string is used to infer decimals, e.g. 3.14 has 2 decimals
        return roq::fix::parse_number((*iter).template get<std::string_view>());
      }
      case number_integer:
      case number_unsigned:
      case number_float:
        // note! undefined decimals, will be rounded by the gateway
        return {(*iter).template get<double>(), {}};
      default:
        throw roq::RuntimeError{"Invalid type for number"sv};
    }
  } else if constexpr (roq::utils::is_any<T, roq::fix::OrdType, roq::fix::Side, roq::fix::TimeInForce>::value) {
    return map<T>((*iter).template get<std::string_view>());
  } else {
    static_assert(roq::utils::always_false<T>, "not implemented for this type");
  }
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(Handler &handler, uint64_t session_id, roq::io::net::tcp::Connection::Factory &factory, Shared &shared)
    : handler_{handler}, session_id_{session_id}, server_{roq::web::rest::ServerFactory::create(*this, factory)},
      shared_{shared} {
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::BusinessMessageReject> const &event) {
  if (zombie())
    return;
  auto &[trace_info, business_message_reject] = event;
  send_text(
      R"({{)"
      R"("jsonrpc":"{}",)"
      R"("method":"business_message_reject",)"
      R"("params":{})"
      R"(}})"sv,
      JSONRPC_VERSION,
      json::BusinessMessageReject{business_message_reject});
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelReject> const &) {
  // XXX TODO send notification
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::ExecutionReport> const &event) {
  if (zombie())
    return;
  auto &[trace_info, execution_report] = event;
  send_text(
      R"({{)"
      R"("jsonrpc":"{}",)"
      R"("method":"execution_report",)"
      R"("params":{})"
      R"(}})"sv,
      JSONRPC_VERSION,
      json::ExecutionReport{execution_report});
}

bool Session::ready() const {
  return state_ == State::READY;
}

bool Session::zombie() const {
  return state_ == State::ZOMBIE;
}

void Session::close() {
  state_ = State::ZOMBIE;
  (*server_).close();
}

// web::rest::Server::Handler

void Session::operator()(roq::web::rest::Server::Disconnected const &) {
  state_ = State::ZOMBIE;
  shared_.session_remove(session_id_);
}

// note!
//   roq::web::rest::Server guarantees that there will always be exactly one response
//   - an automatic "not-found" (404) response is generated if this handler doesn't send anything
//   - an exception is thrown if this handler tries to send more than one response
void Session::operator()(roq::web::rest::Server::Request const &request) {
  if (zombie())
    return;
  auto success = false;
  try {
    if (request.headers.connection == roq::web::http::Connection::UPGRADE) {
      roq::log::info("Upgrading session_id={} to websocket..."sv, session_id_);
      (*server_).upgrade(request);
    } else {
      roq::log::info("DEBUG request={}"sv, request);
      auto path = request.path;  // note! url path has already been split
      if (!std::empty(path) && !std::empty(shared_.settings.json.url_prefix) &&
          path[0] == shared_.settings.json.url_prefix)
        path = path.subspan(1);  // drop prefix
      if (!std::empty(path)) {
        Response response{*server_, request, shared_.encode_buffer};
        route(response, request, path);
      }
    }
    success = true;
  } catch (roq::RuntimeError &e) {
    roq::log::error("Error: {}"sv, e);
  } catch (std::exception &e) {
    roq::log::error("Error: {}"sv, e.what());
  }
  if (!success)
    close();
}

void Session::operator()(roq::web::rest::Server::Text const &text) {
  if (zombie())
    return;
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

void Session::operator()(roq::web::rest::Server::Binary const &) {
  roq::log::warn("Unexpected"sv);
  close();
}

// rest

void Session::route(
    Response &response, roq::web::rest::Server::Request const &request, std::span<std::string_view> const &path) {
  switch (request.method) {
    using enum roq::web::http::Method;
    case GET:
      if (std::size(path) == 1) {
        if (path[0] == "symbols"sv)
          get_symbols(response, request);
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

void Session::get_symbols(Response &response, roq::web::rest::Server::Request const &) {
  if (std::empty(shared_.symbols)) {
    response(roq::web::http::Status::NOT_FOUND, roq::web::http::ContentType::APPLICATION_JSON, "[]"sv);
  } else {
    response(
        roq::web::http::Status::OK,
        roq::web::http::ContentType::APPLICATION_JSON,
        R"(["{}"])"sv,
        fmt::join(shared_.symbols, R"(",")"sv));
  }
}

// ws

// note!
//   using https://www.jsonrpc.org/specification
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

void Session::process_jsonrpc(std::string_view const &method, auto const &params, auto const &id) {
  try {
    roq::TraceInfo trace_info;
    if (method == "logon"sv) {
      if (!ready())
        logon(trace_info, params, id);
      else
        send_error(Error::ALREADY_LOGGED_ON, id);
    } else {
      if (ready()) {
        if (method == "logout"sv)
          logout(trace_info, params, id);
        else if (method == "order_status_request"sv)
          order_status_request(trace_info, params, id);
        else if (method == "new_order_single"sv)
          new_order_single(trace_info, params, id);
        else if (method == "order_cancel_request"sv)
          order_cancel_request(trace_info, params, id);
        else if (method == "order_mass_status_request"sv)
          order_mass_status_request(trace_info, params, id);
        else if (method == "order_mass_cancel_request"sv)
          order_mass_cancel_request(trace_info, params, id);
        else
          send_error(UNKNOWN_METHOD, id);
      } else {
        send_error(Error::NOT_LOGGED_ON, id);
      }
    }
  } catch (roq::oms::NotReady const &) {
    send_error(Error::NOT_READY, id);
  }
}

void Session::logon(roq::TraceInfo const &, auto const &params, auto const &id) {
  assert(state_ == State::WAITING_LOGON);
  auto username = get<std::string_view>(params, "username"sv);
  auto password = get<std::string_view>(params, "password"sv);
  auto success = [&]() {
    state_ = State::READY;
    username_ = username;
    send_result(SUCCESS, id);
  };
  auto failure = [&](auto &reason) { send_error(reason, id); };
  shared_.session_logon(session_id_, username, password, success, failure);
}

void Session::logout(roq::TraceInfo const &, [[maybe_unused]] auto const &params, auto const &id) {
  assert(ready());
  auto success = [&]() {
    if (ready())
      state_ = State::WAITING_LOGON;
    username_.clear();
    send_result(SUCCESS, id);
  };
  auto failure = [&](auto &reason) { send_error(reason, id); };
  shared_.session_logout(session_id_, success, failure);
}

void Session::order_status_request(roq::TraceInfo const &trace_info, auto const &params, auto const &id) {
  auto order_id = get<std::string_view>(params, "order_id"sv);
  auto cl_ord_id = get<std::string_view>(params, "cl_ord_id"sv);
  auto ord_status_req_id = get<std::string_view>(params, "ord_status_req_id"sv);
  auto order_status_request = roq::fix_bridge::fix::OrderStatusRequest{
      .order_id = order_id,
      .cl_ord_id = cl_ord_id,
      .ord_status_req_id = ord_status_req_id,
      .symbol = {},
      .security_exchange = {},
      .side = {},
  };
  roq::log::debug("order_status_request={}"sv, order_status_request);
  dispatch(trace_info, order_status_request);
  send_result(SUCCESS, id);
}

void Session::new_order_single(roq::TraceInfo const &trace_info, auto const &params, auto const &id) {
  auto cl_ord_id = get<std::string_view>(params, "cl_ord_id"sv);
  auto exchange = get<std::string_view>(params, "exchange"sv);
  auto symbol = get<std::string_view>(params, "symbol"sv);
  auto side = get<roq::fix::Side>(params, "side"sv);
  auto quantity = get<roq::utils::Number>(params, "quantity"sv);
  auto ord_type = get<roq::fix::OrdType>(params, "ord_type"sv);
  auto price = get<roq::utils::Number>(params, "price"sv);
  auto stop_px = get<roq::utils::Number>(params, "stop_px"sv);
  auto time_in_force = get<roq::fix::TimeInForce>(params, "time_in_force"sv);
  auto new_order_single = roq::fix_bridge::fix::NewOrderSingle{
      .cl_ord_id = cl_ord_id,
      .no_party_ids = {},
      .account = {},
      .handl_inst = {},
      .exec_inst = {},
      .no_trading_sessions = {},
      .symbol = symbol,
      .security_exchange = exchange,
      .side = side,
      .transact_time = {},
      .order_qty = quantity,
      .ord_type = ord_type,
      .price = price,
      .stop_px = stop_px,
      .time_in_force = time_in_force,
      .text = {},
      .position_effect = {},
      .max_show = {},
  };
  roq::log::debug("new_order_single={}"sv, new_order_single);
  dispatch(trace_info, new_order_single);
  send_result(SUCCESS, id);
}

void Session::order_cancel_request(roq::TraceInfo const &trace_info, auto const &params, auto const &id) {
  auto orig_cl_ord_id = get<std::string_view>(params, "orig_cl_ord_id"sv);
  auto cl_ord_id = get<std::string_view>(params, "cl_ord_id"sv);
  auto exchange = get<std::string_view>(params, "exchange"sv);
  auto symbol = get<std::string_view>(params, "symbol"sv);
  auto order_cancel_request = roq::fix_bridge::fix::OrderCancelRequest{
      .orig_cl_ord_id = orig_cl_ord_id,
      .order_id = {},
      .cl_ord_id = cl_ord_id,
      .symbol = symbol,
      .security_exchange = exchange,
      .side = {},
      .transact_time = {},
      .order_qty = {},
      .text = {},
  };
  roq::log::debug("order_cancel_request={}"sv, order_cancel_request);
  dispatch(trace_info, order_cancel_request);
  send_result(SUCCESS, id);
}

void Session::order_mass_status_request(roq::TraceInfo const &trace_info, auto const &params, auto const &id) {
  auto mass_status_req_id = get<std::string_view>(params, "mass_status_req_id"sv);
  auto order_mass_status_request = roq::fix_bridge::fix::OrderMassStatusRequest{
      .mass_status_req_id = mass_status_req_id,
      .mass_status_req_type = roq::fix::MassStatusReqType::ORDERS,
      .trading_session_id = {},
      .symbol = {},
      .security_exchange = {},
      .side = {},
  };
  roq::log::debug("order_mass_status_request={}"sv, order_mass_status_request);
  dispatch(trace_info, order_mass_status_request);
  send_result(SUCCESS, id);
}

void Session::order_mass_cancel_request(roq::TraceInfo const &trace_info, auto const &params, auto const &id) {
  auto cl_ord_id = get<std::string_view>(params, "cl_ord_id"sv);
  auto order_mass_cancel_request = roq::fix_bridge::fix::OrderMassCancelRequest{
      .cl_ord_id = cl_ord_id,
      .mass_cancel_request_type = roq::fix::MassCancelRequestType::CANCEL_ALL_ORDERS,
      .trading_session_id = {},
      .transact_time = {},
  };
  roq::log::debug("order_mass_cancel_request={}"sv, order_mass_cancel_request);
  dispatch(trace_info, order_mass_cancel_request);
  send_result(SUCCESS, id);
}

void Session::dispatch(roq::TraceInfo const &trace_info, auto const &value) {
  assert(!std::empty(username_));
  roq::Trace event{trace_info, value};
  handler_(event, username_);
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

}  // namespace json
}  // namespace simple
