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

struct Controller final : public client::Handler {
  Controller(client::Dispatcher &, Settings const &, Config const &, roq::io::Context &context);

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

  void publish_accounts();
  void publish_users();

  void load_trades();

 private:
  client::Dispatcher &dispatcher_;
  io::Context &context_;
  std::unique_ptr<database::Session> database_;
  control::Manager control_manager_;
  Shared shared_;
  // state
  UUID last_session_id_ = {};
  uint64_t last_seqno_ = {};
  bool ready_ = {};
  // control
  absl::flat_hash_set<std::string> latch_by_account_;
  // buffering
  std::vector<RiskLimit> risk_limits_buffer_;
  std::vector<database::Trade> trades_buffer_;
};

}  // namespace risk_manager
}  // namespace roq
