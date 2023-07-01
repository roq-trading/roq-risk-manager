/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <memory>
#include <vector>

#include "roq/api.hpp"

#include "roq/io/context.hpp"

#include "roq/io/web/uri.hpp"

#include "roq/io/net/connection_factory.hpp"
#include "roq/io/net/connection_manager.hpp"

#include "roq/fix/message.hpp"

#include "roq/fix_bridge/fix/business_message_reject.hpp"
#include "roq/fix_bridge/fix/execution_report.hpp"
#include "roq/fix_bridge/fix/heartbeat.hpp"
#include "roq/fix_bridge/fix/logon.hpp"
#include "roq/fix_bridge/fix/logout.hpp"
#include "roq/fix_bridge/fix/market_data_incremental_refresh.hpp"
#include "roq/fix_bridge/fix/market_data_request_reject.hpp"
#include "roq/fix_bridge/fix/market_data_snapshot_full_refresh.hpp"
#include "roq/fix_bridge/fix/new_order_single.hpp"
#include "roq/fix_bridge/fix/order_cancel_reject.hpp"
#include "roq/fix_bridge/fix/order_cancel_replace_request.hpp"
#include "roq/fix_bridge/fix/order_cancel_request.hpp"
#include "roq/fix_bridge/fix/order_mass_cancel_request.hpp"
#include "roq/fix_bridge/fix/order_mass_status_request.hpp"
#include "roq/fix_bridge/fix/order_status_request.hpp"
#include "roq/fix_bridge/fix/reject.hpp"
#include "roq/fix_bridge/fix/resend_request.hpp"
#include "roq/fix_bridge/fix/security_definition.hpp"
#include "roq/fix_bridge/fix/security_list.hpp"
#include "roq/fix_bridge/fix/test_request.hpp"

#include "simple/settings.hpp"
#include "simple/shared.hpp"

namespace simple {
namespace fix {

// note! supports both rest and websocket

struct Session final : public roq::io::net::ConnectionManager::Handler {
  struct Handler {
    virtual void operator()(roq::Trace<roq::fix_bridge::fix::SecurityDefinition> const &) = 0;
    virtual void operator()(
        roq::Trace<roq::fix_bridge::fix::BusinessMessageReject> const &, std::string_view const &username) = 0;
    virtual void operator()(
        roq::Trace<roq::fix_bridge::fix::OrderCancelReject> const &, std::string_view const &username) = 0;
    virtual void operator()(
        roq::Trace<roq::fix_bridge::fix::ExecutionReport> const &, std::string_view const &username) = 0;
  };

  Session(
      Handler &,
      Settings const &,
      roq::io::Context &,
      Shared &,
      roq::io::web::URI const &,
      std::string_view const &username,
      std::string_view const &password);

  void operator()(roq::Event<roq::Start> const &);
  void operator()(roq::Event<roq::Stop> const &);
  void operator()(roq::Event<roq::Timer> const &);

  bool ready() const;

  void operator()(roq::Trace<roq::fix_bridge::fix::OrderStatusRequest> const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::NewOrderSingle> const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelReplaceRequest> const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelRequest> const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::OrderMassStatusRequest> const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::OrderMassCancelRequest> const &);

 private:
  enum class State;

 protected:
  void operator()(State);

  // io::net::ConnectionManager::Handler
  void operator()(roq::io::net::ConnectionManager::Connected const &) override;
  void operator()(roq::io::net::ConnectionManager::Disconnected const &) override;
  void operator()(roq::io::net::ConnectionManager::Read const &) override;

  // inbound

  void check(roq::fix::Header const &);

  void parse(roq::Trace<roq::fix::Message> const &);

  template <typename T>
  void dispatch(roq::Trace<roq::fix::Message> const &, T const &);

  // - session

  void operator()(roq::Trace<roq::fix_bridge::fix::Reject> const &, roq::fix::Header const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::ResendRequest> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::Logon> const &, roq::fix::Header const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::Logout> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::Heartbeat> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::TestRequest> const &, roq::fix::Header const &);

  // - business

  void operator()(roq::Trace<roq::fix_bridge::fix::BusinessMessageReject> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::SecurityList> const &, roq::fix::Header const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::SecurityDefinition> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::MarketDataRequestReject> const &, roq::fix::Header const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::MarketDataSnapshotFullRefresh> const &, roq::fix::Header const &);
  void operator()(roq::Trace<roq::fix_bridge::fix::MarketDataIncrementalRefresh> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::OrderCancelReject> const &, roq::fix::Header const &);

  void operator()(roq::Trace<roq::fix_bridge::fix::ExecutionReport> const &, roq::fix::Header const &);

  // outbound

  template <typename T>
  void send(T const &value);

  template <typename T>
  void send_helper(T const &value);

  void send_logon();
  void send_logout(std::string_view const &text);
  void send_heartbeat(std::string_view const &test_req_id);
  void send_test_request(std::chrono::nanoseconds now);

  void send_security_list_request();
  void send_security_definition_request(std::string_view const &exchange, std::string_view const &symbol);

  void send_market_data_request(std::string_view const &exchange, std::string_view const &symbol);

  // download

  void download_security_list();

 private:
  Handler &handler_;
  Shared &shared_;
  // config
  std::string_view const username_;
  std::string_view const password_;
  std::string_view const sender_comp_id_;
  std::string_view const target_comp_id_;
  std::chrono::nanoseconds const ping_freq_;
  bool const debug_;
  uint32_t const market_depth_;
  // connection
  std::unique_ptr<roq::io::net::ConnectionFactory> const connection_factory_;
  std::unique_ptr<roq::io::net::ConnectionManager> const connection_manager_;
  // messaging
  struct {
    uint64_t msg_seq_num = {};
  } inbound_;
  struct {
    uint64_t msg_seq_num = {};
  } outbound_;
  std::vector<std::byte> decode_buffer_;
  std::vector<std::byte> encode_buffer_;
  // state
  enum class State {
    DISCONNECTED,
    LOGON_SENT,
    GET_SECURITY_LIST,
    READY,
  } state_ = {};
  std::chrono::nanoseconds next_heartbeat_ = {};
  absl::flat_hash_map<std::string, absl::flat_hash_set<std::string>> exchange_symbols_;
  // TEST
  bool const disable_market_data_;
};

}  // namespace fix
}  // namespace simple
