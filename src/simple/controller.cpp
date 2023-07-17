/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/controller.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "tools/simple.hpp"

using namespace std::literals;

namespace simple {

Controller::Controller(roq::client::Dispatcher &dispatcher, Settings const &, Config const &config)
    : dispatcher_{dispatcher}, shared_{config} {
}

void Controller::operator()(roq::Event<roq::Timer> const &) {
  auto callback = [&](auto &account) {
    risk_limit_buffer_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = roq::RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .buy_limit = position.buy_limit(),
          .sell_limit = position.sell_limit(),
      };
      risk_limit_buffer_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish(account.name, callback);
    if (!std::empty(risk_limit_buffer_)) {
      auto risk_limits = roq::RiskLimits{
          .label = "test"sv,  // XXX
          .account = account.name,
          .user = {},
          .limits = risk_limit_buffer_,
          .seqno = {},  // XXX
      };
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_accounts(callback);
}

void Controller::operator()(roq::Event<roq::Connected> const &) {
}

void Controller::operator()(roq::Event<roq::Disconnected> const &) {
  published_accounts_.clear();
}

void Controller::operator()(roq::Event<roq::DownloadBegin> const &event) {
  auto &[message_info, download_begin] = event;
  if (std::empty(download_begin.account))
    return;
  roq::log::warn(R"(*** DOWNLOAD NOW IN PROGRESS *** (account="{}"))"sv, download_begin.account);
}

void Controller::operator()(roq::Event<roq::DownloadEnd> const &event) {
  auto &[message_info, download_end] = event;
  if (std::empty(download_end.account))
    return;
  roq::log::warn(R"(*** DOWNLOAD HAS COMPLETED *** (account="{}"))"sv, download_end.account);
  auto res = published_accounts_.emplace(download_end.account);
  if (res.second)
    shared_.publish(download_end.account);
}

void Controller::operator()(roq::Event<roq::GatewayStatus> const &) {
}

void Controller::operator()(roq::Event<roq::ReferenceData> const &event) {
  auto &[message_info, reference_data] = event;
  auto &instrument = shared_.get_instrument(reference_data.exchange, reference_data.symbol);
  if (instrument(reference_data)) {
    auto callback = [&](auto &account) { account(reference_data); };
    shared_.get_all_accounts(callback);
  }
}

void Controller::operator()(roq::Event<roq::OrderUpdate> const &event) {
  roq::log::info("event={}"sv, event);
}

void Controller::operator()(roq::Event<roq::TradeUpdate> const &event) {
  roq::log::info("event={}"sv, event);
  auto &[message_info, trade_update] = event;
  auto callback = [&](auto &account) { account(trade_update); };
  shared_.get_account(trade_update.account, callback);
}

}  // namespace simple