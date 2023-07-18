/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <vector>

#include "roq/client.hpp"

#include "simple/config.hpp"
#include "simple/settings.hpp"
#include "simple/shared.hpp"

namespace simple {

struct Controller final : public roq::client::Handler {
  Controller(roq::client::Dispatcher &, Settings const &, Config const &);

  Controller(Controller &&) = default;
  Controller(Controller const &) = delete;

 protected:
  void operator()(roq::Event<roq::Timer> const &) override;
  void operator()(roq::Event<roq::Connected> const &) override;
  void operator()(roq::Event<roq::Disconnected> const &) override;
  void operator()(roq::Event<roq::DownloadBegin> const &) override;
  void operator()(roq::Event<roq::DownloadEnd> const &) override;
  void operator()(roq::Event<roq::GatewayStatus> const &) override;
  void operator()(roq::Event<roq::ReferenceData> const &) override;
  void operator()(roq::Event<roq::OrderUpdate> const &) override;
  void operator()(roq::Event<roq::TradeUpdate> const &) override;

  void publish_accounts();

 private:
  roq::client::Dispatcher &dispatcher_;
  Shared shared_;
  absl::flat_hash_set<std::string> published_accounts_;
  std::vector<roq::RiskLimit> limits_;
  uint64_t last_seqno_ = {};
};

}  // namespace simple
