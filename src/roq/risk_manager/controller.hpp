/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <vector>

#include "roq/client.hpp"

#include "roq/io/context.hpp"

#include "roq/risk_manager/config.hpp"
#include "roq/risk_manager/settings.hpp"
#include "roq/risk_manager/shared.hpp"

#include "roq/risk_manager/database/session.hpp"

#include "roq/risk_manager/control/manager.hpp"

namespace roq {
namespace risk_manager {

struct Controller final : public client::Handler, public control::Manager::Handler {
  Controller(client::Dispatcher &, Settings const &, Config const &, roq::io::Context &context, size_t source_count);

  Controller(Controller &&) = default;
  Controller(Controller const &) = delete;

 protected:
  // client::Handler
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;
  void operator()(Event<DownloadBegin> const &) override;
  void operator()(Event<DownloadEnd> const &) override;
  void operator()(Event<Ready> const &) override;
  void operator()(Event<ReferenceData> const &) override;
  void operator()(Event<TradeUpdate> const &) override;
  void operator()(Event<PositionUpdate> const &) override;
  void operator()(Event<FundsUpdate> const &) override;

  // control::Manager::Handler
  // note! empty

  void operator()(MessageInfo const &);

  void publish_accounts(uint8_t source);
  void publish_users(uint8_t source);

  void load_positions();

 private:
  client::Dispatcher &dispatcher_;
  io::Context &context_;
  std::unique_ptr<database::Session> database_;
  control::Manager control_manager_;
  Shared shared_;
  // time
  std::chrono::nanoseconds last_exchange_time_utc_ = {};
  // sources
  struct State final {
    UUID session_id;
    uint64_t seqno = {};
    bool ready = {};
    absl::flat_hash_set<std::string> latch_by_account;
  };
  std::vector<State> state_;
  // buffering
  std::vector<RiskLimit> risk_limits_buffer_;
  std::vector<database::Trade> trades_buffer_;
};

}  // namespace risk_manager
}  // namespace roq
