/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/fix/session.hpp"

#include "roq/logging.hpp"

#include "roq/oms/exceptions.hpp"

#include "roq/utils/chrono.hpp"
#include "roq/utils/traits.hpp"
#include "roq/utils/update.hpp"

#include "roq/debug/fix/message.hpp"
#include "roq/debug/hex/message.hpp"

#include "roq/fix/reader.hpp"

#include "roq/fix_bridge/fix/market_data_request.hpp"
#include "roq/fix_bridge/fix/new_order_single.hpp"
#include "roq/fix_bridge/fix/order_cancel_request.hpp"
#include "roq/fix_bridge/fix/security_definition_request.hpp"
#include "roq/fix_bridge/fix/security_list_request.hpp"

using namespace std::literals;

namespace simple {
namespace fix {

// === CONSTANTS ===

namespace {
auto const FIX_VERSION = roq::fix::Version::FIX_44;
auto const LOGOUT_RESPONSE = "LOGOUT"sv;
}  // namespace

// === HELPERS ===

namespace {
auto create_connection_factory(auto &settings, auto &context, auto &uri) {
  auto config = roq::io::net::ConnectionFactory::Config{
      .interface = {},
      .uris = {&uri, 1},
      .validate_certificate = settings.net.tls_validate_certificate,
  };
  return roq::io::net::ConnectionFactory::create(context, config);
}

auto create_connection_manager(auto &handler, auto &settings, auto &connection_factory) {
  auto config = roq::io::net::ConnectionManager::Config{
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
  };
  return roq::io::net::ConnectionManager::create(handler, connection_factory, config);
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(
    Handler &handler,
    Settings const &settings,
    roq::io::Context &context,
    Shared &shared,
    roq::io::web::URI const &uri,
    std::string_view const &username,
    std::string_view const &password)
    : handler_{handler}, shared_{shared}, username_{username}, password_{password},
      sender_comp_id_{settings.fix.sender_comp_id}, target_comp_id_{settings.fix.target_comp_id},
      ping_freq_{settings.fix.ping_freq}, debug_{settings.fix.debug}, market_depth_{settings.fix.market_depth},
      connection_factory_{create_connection_factory(settings, context, uri)},
      connection_manager_{create_connection_manager(*this, settings, *connection_factory_)},
      decode_buffer_(settings.fix.decode_buffer_size), encode_buffer_(settings.fix.encode_buffer_size),
      disable_market_data_{settings.test.disable_market_data} {
}

void Session::operator()(roq::Event<roq::Start> const &) {
  (*connection_manager_).start();
}

void Session::operator()(roq::Event<roq::Stop> const &) {
  (*connection_manager_).stop();
}

void Session::operator()(roq::Event<roq::Timer> const &event) {
  auto now = event.value.now;
  (*connection_manager_).refresh(now);
  if (state_ <= State::LOGON_SENT)
    return;
  if (next_heartbeat_ <= now) {
    next_heartbeat_ = now + ping_freq_;
    send_test_request(now);
  }
}

bool Session::ready() const {
  return state_ == State::READY;
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderStatusRequest> const &event) {
  send(event);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::NewOrderSingle> const &event) {
  send(event);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelReplaceRequest> const &event) {
  send(event);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelRequest> const &event) {
  send(event);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderMassStatusRequest> const &event) {
  send(event);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderMassCancelRequest> const &event) {
  send(event);
}

void Session::operator()(Session::State state) {
  if (roq::utils::update(state_, state))
    roq::log::debug("state={}"sv, magic_enum::enum_name(state));
}

// io::net::ConnectionManager::Handler

void Session::operator()(roq::io::net::ConnectionManager::Connected const &) {
  roq::log::debug("Connected"sv);
  send_logon();
  (*this)(State::LOGON_SENT);
}

void Session::operator()(roq::io::net::ConnectionManager::Disconnected const &) {
  roq::log::debug("Disconnected"sv);
  outbound_ = {};
  inbound_ = {};
  next_heartbeat_ = {};
  exchange_symbols_.clear();
  (*this)(State::DISCONNECTED);
}

void Session::operator()(roq::io::net::ConnectionManager::Read const &) {
  auto logger = [this](auto &message) {
    if (debug_) [[unlikely]]
      roq::log::info("{}"sv, roq::debug::fix::Message{message});
  };
  auto buffer = (*connection_manager_).buffer();
  size_t total_bytes = 0;
  while (!std::empty(buffer)) {
    roq::TraceInfo trace_info;
    auto parser = [&](auto &message) {
      try {
        check(message.header);
        roq::Trace event{trace_info, message};
        parse(event);
      } catch (std::exception &) {
        roq::log::warn("{}"sv, roq::debug::fix::Message{buffer});
#ifndef NDEBUG
        roq::log::warn("{}"sv, roq::debug::hex::Message{buffer});
#endif
        roq::log::error("Message could not be parsed. PLEASE REPORT!"sv);
        throw;
      }
    };
    auto bytes = roq::fix::Reader<FIX_VERSION>::dispatch(buffer, parser, logger);
    if (bytes == 0)
      break;
    assert(bytes <= std::size(buffer));
    total_bytes += bytes;
    buffer = buffer.subspan(bytes);
  }
  (*connection_manager_).drain(total_bytes);
}

// inbound

void Session::check(roq::fix::Header const &header) {
  auto current = header.msg_seq_num;
  auto expected = inbound_.msg_seq_num + 1;
  if (current != expected) [[unlikely]] {
    if (expected < current) {
      roq::log::warn(
          "*** SEQUENCE GAP *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          current - inbound_.msg_seq_num);
    } else {
      roq::log::warn(
          "*** SEQUENCE REPLAY *** "
          "current={} previous={} distance={}"sv,
          current,
          inbound_.msg_seq_num,
          inbound_.msg_seq_num - current);
    }
  }
  inbound_.msg_seq_num = current;
}

void Session::parse(roq::Trace<roq::fix::Message> const &event) {
  auto &[trace_info, message] = event;
  auto &header = message.header;
  switch (header.msg_type) {
    using enum roq::fix::MsgType;
    case REJECT: {
      auto reject = roq::fix_bridge::fix::Reject::create(message);
      dispatch(event, reject);
      break;
    }
    case RESEND_REQUEST: {
      auto resend_request = roq::fix_bridge::fix::ResendRequest::create(message);
      dispatch(event, resend_request);
      break;
    }
    case LOGON: {
      auto logon = roq::fix_bridge::fix::Logon::create(message);
      dispatch(event, logon);
      break;
    }
    case LOGOUT: {
      auto logout = roq::fix_bridge::fix::Heartbeat::create(message);
      dispatch(event, logout);
      break;
    }
    case HEARTBEAT: {
      auto heartbeat = roq::fix_bridge::fix::Heartbeat::create(message);
      dispatch(event, heartbeat);
      break;
    }
    case TEST_REQUEST: {
      auto test_request = roq::fix_bridge::fix::TestRequest::create(message);
      dispatch(event, test_request);
      break;
    }
    case BUSINESS_MESSAGE_REJECT: {
      auto business_message_reject = roq::fix_bridge::fix::BusinessMessageReject::create(message);
      dispatch(event, business_message_reject);
      break;
    }
    case SECURITY_LIST: {
      auto security_list = roq::fix_bridge::fix::SecurityList::create(message, decode_buffer_);
      dispatch(event, security_list);
      break;
    }
    case SECURITY_DEFINITION: {
      auto security_definition = roq::fix_bridge::fix::SecurityDefinition::create(message, decode_buffer_);
      dispatch(event, security_definition);
      break;
    }
    case MARKET_DATA_REQUEST_REJECT: {
      auto market_data_request_reject = roq::fix_bridge::fix::MarketDataRequestReject::create(message, decode_buffer_);
      dispatch(event, market_data_request_reject);
      break;
    }
    case MARKET_DATA_SNAPSHOT_FULL_REFRESH: {
      auto market_data_snapshot_full_refresh =
          roq::fix_bridge::fix::MarketDataSnapshotFullRefresh::create(message, decode_buffer_);
      dispatch(event, market_data_snapshot_full_refresh);
      break;
    }
    case MARKET_DATA_INCREMENTAL_REFRESH: {
      auto market_data_incremental_refresh =
          roq::fix_bridge::fix::MarketDataIncrementalRefresh::create(message, decode_buffer_);
      dispatch(event, market_data_incremental_refresh);
      break;
    }
    case ORDER_CANCEL_REJECT: {
      auto order_cancel_reject = roq::fix_bridge::fix::OrderCancelReject::create(message, decode_buffer_);
      dispatch(event, order_cancel_reject);
      break;
    }
    case EXECUTION_REPORT: {
      auto execution_report = roq::fix_bridge::fix::ExecutionReport::create(message, decode_buffer_);
      dispatch(event, execution_report);
      break;
    }
    default:
      roq::log::warn("Unexpected msg_type={}"sv, header.msg_type);
  }
}

template <typename T>
void Session::dispatch(roq::Trace<roq::fix::Message> const &event, T const &value) {
  auto &[trace_info, message] = event;
  roq::Trace event_2{trace_info, value};
  (*this)(event_2, message.header);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::Reject> const &event, roq::fix::Header const &) {
  auto &[trace_info, reject] = event;
  roq::log::debug("reject={}, trace_info={}"sv, reject, trace_info);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::ResendRequest> const &event, roq::fix::Header const &) {
  auto &[trace_info, resend_request] = event;
  roq::log::debug("resend_request={}, trace_info={}"sv, resend_request, trace_info);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::Logon> const &event, roq::fix::Header const &) {
  auto &[trace_info, logon] = event;
  roq::log::debug("logon={}, trace_info={}"sv, logon, trace_info);
  assert(state_ == State::LOGON_SENT);
  download_security_list();
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::Logout> const &event, roq::fix::Header const &) {
  auto &[trace_info, logout] = event;
  roq::log::debug("logout={}, trace_info={}"sv, logout, trace_info);
  // note! mandated, must send a logout response
  send_logout(LOGOUT_RESPONSE);
  roq::log::warn("closing connection"sv);
  (*connection_manager_).close();
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::Heartbeat> const &event, roq::fix::Header const &) {
  auto &[trace_info, heartbeat] = event;
  roq::log::debug("heartbeat={}, trace_info={}"sv, heartbeat, trace_info);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::TestRequest> const &event, roq::fix::Header const &) {
  auto &[trace_info, test_request] = event;
  roq::log::debug("test_request={}, trace_info={}"sv, test_request, trace_info);
  send_heartbeat(test_request.test_req_id);
}

void Session::operator()(
    roq::Trace<roq::fix_bridge::fix::BusinessMessageReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, business_message_reject] = event;
  roq::log::debug("business_message_reject={}, trace_info={}"sv, business_message_reject, trace_info);
  handler_(event, username_);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::SecurityList> const &event, roq::fix::Header const &) {
  auto &[trace_info, security_list] = event;
  roq::log::debug("security_list={}, trace_info={}"sv, security_list, trace_info);
  assert(state_ == State::GET_SECURITY_LIST);
  for (auto &item : security_list.no_related_sym) {
    if (shared_.include(item.symbol)) {
      exchange_symbols_[item.security_exchange].emplace(item.symbol);
      send_security_definition_request(item.security_exchange, item.symbol);
      if (!disable_market_data_)
        send_market_data_request(item.security_exchange, item.symbol);
    }
  }
  (*this)(State::READY);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::SecurityDefinition> const &event, roq::fix::Header const &) {
  auto &[trace_info, security_definition] = event;
  roq::log::debug("security_definition={}, trace_info={}"sv, security_definition, trace_info);
  handler_(event);
}

void Session::operator()(
    roq::Trace<roq::fix_bridge::fix::MarketDataRequestReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_request_reject] = event;
  roq::log::debug("market_data_request_reject={}, trace_info={}"sv, market_data_request_reject, trace_info);
}

void Session::operator()(
    roq::Trace<roq::fix_bridge::fix::MarketDataSnapshotFullRefresh> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_snapshot_full_refresh] = event;
  roq::log::debug(
      "market_data_snapshot_full_refresh={}, trace_info={}"sv, market_data_snapshot_full_refresh, trace_info);
  // XXX must forward
}

void Session::operator()(
    roq::Trace<roq::fix_bridge::fix::MarketDataIncrementalRefresh> const &event, roq::fix::Header const &) {
  auto &[trace_info, market_data_incremental_refresh] = event;
  roq::log::debug("market_data_incremental_refresh={}, trace_info={}"sv, market_data_incremental_refresh, trace_info);
  // XXX must forward
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelReject> const &event, roq::fix::Header const &) {
  auto &[trace_info, order_cancel_reject] = event;
  roq::log::debug("order_cancel_reject={}, trace_info={}"sv, order_cancel_reject, trace_info);
  handler_(event, username_);
}

void Session::operator()(roq::Trace<roq::fix_bridge::fix::ExecutionReport> const &event, roq::fix::Header const &) {
  auto &[trace_info, execution_report] = event;
  roq::log::debug("execution_report={}, trace_info={}"sv, execution_report, trace_info);
  handler_(event, username_);
}

// outbound

template <typename T>
void Session::send(T const &value) {
  if constexpr (roq::utils::is_specialization<T, roq::Trace>::value) {
    // external
    if (!ready())
      throw roq::oms::NotReady{"not ready"sv};
    send_helper(value.value);
  } else {
    // internal
    send_helper(value);
  }
}

template <typename T>
void Session::send_helper(T const &value) {
  auto sending_time = roq::clock::get_realtime();
  auto header = roq::fix::Header{
      .version = FIX_VERSION,
      .msg_type = T::MSG_TYPE,
      .sender_comp_id = sender_comp_id_,
      .target_comp_id = target_comp_id_,
      .msg_seq_num = ++outbound_.msg_seq_num,  // note!
      .sending_time = sending_time,
  };
  auto message = value.encode(header, encode_buffer_);
  if (debug_) [[unlikely]]
    roq::log::info("{}"sv, roq::debug::fix::Message{message});
  (*connection_manager_).send(message);
}

void Session::send_logon() {
  auto heart_bt_int = static_cast<decltype(roq::fix_bridge::fix::Logon::heart_bt_int)>(
      std::chrono::duration_cast<std::chrono::seconds>(ping_freq_).count());
  auto logon = roq::fix_bridge::fix::Logon{
      .encrypt_method = {},
      .heart_bt_int = heart_bt_int,
      .reset_seq_num_flag = true,
      .next_expected_msg_seq_num = inbound_.msg_seq_num + 1,  // note!
      .username = username_,
      .password = password_,
  };
  send(logon);
}

void Session::send_logout(std::string_view const &text) {
  auto logout = roq::fix_bridge::fix::Logout{
      .text = text,
  };
  send(logout);
}

void Session::send_heartbeat(std::string_view const &test_req_id) {
  auto heartbeat = roq::fix_bridge::fix::Heartbeat{
      .test_req_id = test_req_id,
  };
  send(heartbeat);
}

void Session::send_test_request(std::chrono::nanoseconds now) {
  auto test_req_id = fmt::format("{}"sv, now.count());
  auto test_request = roq::fix_bridge::fix::TestRequest{
      .test_req_id = test_req_id,
  };
  send(test_request);
}

void Session::send_security_list_request() {
  auto security_list_request = roq::fix_bridge::fix::SecurityListRequest{
      .security_req_id = "test"sv,
      .security_list_request_type = roq::fix::SecurityListRequestType::ALL_SECURITIES,
      .symbol = {},
      .security_exchange = {},
      .trading_session_id = {},
      .subscription_request_type = roq::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
  };
  send(security_list_request);
}

void Session::send_security_definition_request(std::string_view const &exchange, std::string_view const &symbol) {
  auto security_definition_request = roq::fix_bridge::fix::SecurityDefinitionRequest{
      .security_req_id = "test"sv,
      .security_request_type = roq::fix::SecurityRequestType::REQUEST_LIST_SECURITIES,
      .symbol = symbol,
      .security_exchange = exchange,
      .trading_session_id = {},
      .subscription_request_type = roq::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
  };
  send(security_definition_request);
}

void Session::send_market_data_request(std::string_view const &exchange, std::string_view const &symbol) {
  auto md_entry_types = std::array<roq::fix_bridge::fix::MDReq, 2>{{
      {.md_entry_type = roq::fix::MDEntryType::BID},
      {.md_entry_type = roq::fix::MDEntryType::OFFER},
  }};
  auto related_sym = std::array<roq::fix_bridge::fix::InstrmtMDReq, 1>{{
      {
          .symbol = symbol,
          .security_exchange = exchange,
      },
  }};
  auto market_data_request = roq::fix_bridge::fix::MarketDataRequest{
      .md_req_id = "test"sv,
      .subscription_request_type = roq::fix::SubscriptionRequestType::SNAPSHOT_UPDATES,
      .market_depth = market_depth_,
      .md_update_type = roq::fix::MDUpdateType::INCREMENTAL_REFRESH,
      .aggregated_book = true,  // note! false=MbO, true=MbP
      .no_md_entry_types = md_entry_types,
      .no_related_sym = related_sym,
      .no_trading_sessions = {},
      // note! following fields used to support custom calculations, e.g. vwap
      .custom_type = {},
      .custom_value = {},
  };
  send(market_data_request);
}

// download

void Session::download_security_list() {
  send_security_list_request();
  (*this)(State::GET_SECURITY_LIST);
}

}  // namespace fix
}  // namespace simple
