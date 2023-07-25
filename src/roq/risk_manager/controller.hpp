/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <vector>

#include "roq/client.hpp"

#include "roq/risk_manager/config.hpp"
#include "roq/risk_manager/settings.hpp"
#include "roq/risk_manager/shared.hpp"

namespace roq {
namespace risk_manager {

struct Controller final : public client::Handler {
  Controller(client::Dispatcher &, Settings const &, Config const &);

  Controller(Controller &&) = default;
  Controller(Controller const &) = delete;

 protected:
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;
  void operator()(Event<DownloadBegin> const &) override;
  void operator()(Event<DownloadEnd> const &) override;
  void operator()(Event<Ready> const &) override;
  void operator()(Event<ReferenceData> const &) override;
  void operator()(Event<OrderUpdate> const &) override;
  void operator()(Event<TradeUpdate> const &) override;

  void publish_accounts();
  void publish_users();

 private:
  client::Dispatcher &dispatcher_;
  Shared shared_;
  absl::flat_hash_set<std::string> published_accounts_;
  std::vector<RiskLimit> limits_;
  UUID last_session_id_ = {};
  uint64_t last_seqno_ = {};
  bool ready_ = {};
};

}  // namespace risk_manager
}  // namespace roq
