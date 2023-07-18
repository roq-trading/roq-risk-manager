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

// note! timer is used to achieve batching of updates => gateway & clients can proxy until updates arrive
void Controller::operator()(roq::Event<roq::Timer> const &) {
  if (!ready_)  // note! wait until all accounts have been downloaded
    return;
  publish_accounts();
  publish_users();
}

void Controller::operator()(roq::Event<roq::Connected> const &) {
}

void Controller::operator()(roq::Event<roq::Disconnected> const &) {
  published_accounts_.clear();
  last_seqno_ = {};
  ready_ = {};
  roq::log::warn("*** NOT READY ***"sv);
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
  last_seqno_ = message_info.source_seqno;
  auto res = published_accounts_.emplace(download_end.account);
  if (res.second)
    shared_.publish_account(download_end.account);
}

// note! final download message allows us to publish
void Controller::operator()(roq::Event<roq::GatewayStatus> const &event) {
  auto &[message_info, gateway_status] = event;
  auto callback = [&](auto &user) { shared_.publish_user(user.name); };
  shared_.get_all_users(callback);
  if (!ready_) {
    ready_ = true;
    roq::log::warn("*** READY ***"sv);
  }
}

void Controller::operator()(roq::Event<roq::ReferenceData> const &event) {
  auto &[message_info, reference_data] = event;
  auto &instrument = shared_.get_instrument(reference_data.exchange, reference_data.symbol);
  if (instrument(reference_data)) {
    auto callback = [&](auto &item) { item(reference_data); };
    shared_.get_all_accounts(callback);
    shared_.get_all_users(callback);
  }
}

void Controller::operator()(roq::Event<roq::OrderUpdate> const &event) {
  roq::log::info("event={}"sv, event);
}

void Controller::operator()(roq::Event<roq::TradeUpdate> const &event) {
  roq::log::info("event={}"sv, event);
  auto &[message_info, trade_update] = event;
  last_seqno_ = message_info.source_seqno;
  auto callback = [&](auto &item) { item(trade_update); };
  shared_.get_account(trade_update.account, callback);
  shared_.get_user(trade_update.user, callback);
}

void Controller::publish_accounts() {
  auto callback = [&](auto &account) {
    limits_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = roq::RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_quantity = position.long_quantity(),
          .short_quantity = position.short_quantity(),
          .buy_limit = position.buy_limit(),
          .sell_limit = position.sell_limit(),
      };
      limits_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_account(account.name, callback);
    if (!std::empty(limits_)) {
      auto risk_limits = roq::RiskLimits{
          .label = "test"sv,  // XXX
          .account = account.name,
          .user = {},
          .limits = limits_,
          .seqno = last_seqno_,
      };
      roq::log::debug("{}"sv, risk_limits);
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_accounts(callback);
}

void Controller::publish_users() {
  auto callback = [&](auto &user) {
    limits_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = roq::RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_quantity = position.long_quantity(),
          .short_quantity = position.short_quantity(),
          .buy_limit = position.buy_limit(),
          .sell_limit = position.sell_limit(),
      };
      limits_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_user(user.name, callback);
    if (!std::empty(limits_)) {
      auto risk_limits = roq::RiskLimits{
          .label = "test"sv,  // XXX
          .account = {},
          .user = user.name,
          .limits = limits_,
          .seqno = last_seqno_,
      };
      roq::log::debug("{}"sv, risk_limits);
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_users(callback);
}

}  // namespace simple
