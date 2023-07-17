/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

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

 private:
  roq::client::Dispatcher &dispatcher_;
  Shared shared_;
};

}  // namespace simple
