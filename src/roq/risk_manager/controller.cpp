/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/controller.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "tools/simple.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

Controller::Controller(client::Dispatcher &dispatcher, Settings const &, Config const &config)
    : dispatcher_{dispatcher}, shared_{config} {
}

// note! timer is used to achieve batching of updates => gateway & clients can proxy until updates arrive
void Controller::operator()(Event<Timer> const &) {
  if (!ready_)  // note! wait until all accounts have been downloaded
    return;
  publish_accounts();
  publish_users();
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
  published_accounts_.clear();
  last_session_id_ = {};
  last_seqno_ = {};
  ready_ = {};
  log::warn("*** NOT READY ***"sv);
}

void Controller::operator()(Event<DownloadBegin> const &event) {
  auto &[message_info, download_begin] = event;
  if (std::empty(download_begin.account))
    return;
  log::warn(R"(*** DOWNLOAD NOW IN PROGRESS *** (account="{}"))"sv, download_begin.account);
}

void Controller::operator()(Event<DownloadEnd> const &event) {
  auto &[message_info, download_end] = event;
  if (std::empty(download_end.account))
    return;
  log::warn(R"(*** DOWNLOAD HAS COMPLETED *** (account="{}"))"sv, download_end.account);
  last_session_id_ = message_info.source_session_id;
  last_seqno_ = message_info.source_seqno;
  auto res = published_accounts_.emplace(download_end.account);
  if (res.second)
    shared_.publish_account(download_end.account);
}

void Controller::operator()(Event<Ready> const &) {
  if (ready_)
    return;
  ready_ = true;
  log::warn("*** READY ***"sv);
  auto callback = [&](auto &user) { shared_.publish_user(user.name); };
  shared_.get_all_users(callback);
}

// XXX TODO also use MarketByPrice in case reference data not available...?
void Controller::operator()(Event<ReferenceData> const &event) {
  auto &[message_info, reference_data] = event;
  log::debug("reference_data={}"sv, reference_data);
  auto &instrument = shared_.get_instrument(reference_data.exchange, reference_data.symbol);
  if (instrument(reference_data)) {
    auto callback = [&](auto &item) { item(reference_data); };
    shared_.get_all_accounts(callback);
    shared_.get_all_users(callback);
  }
}

void Controller::operator()(Event<OrderUpdate> const &event) {
  log::info("event={}"sv, event);
}

void Controller::operator()(Event<TradeUpdate> const &event) {
  log::info("event={}"sv, event);
  auto &[message_info, trade_update] = event;
  last_session_id_ = message_info.source_session_id;
  last_seqno_ = message_info.source_seqno;
  auto callback = [&](auto &item) { item(trade_update); };
  shared_.get_account(trade_update.account, callback);
  shared_.get_user(trade_update.user, callback);
}

void Controller::publish_accounts() {
  auto callback = [&](auto &account) {
    limits_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
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
      auto risk_limits = RiskLimits{
          .label = "test"sv,  // XXX
          .account = account.name,
          .user = {},
          .limits = limits_,
          .session_id = last_session_id_,
          .seqno = last_seqno_,
      };
      log::debug("risk_limits={}"sv, risk_limits);
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_accounts(callback);
}

void Controller::publish_users() {
  auto callback = [&](auto &user) {
    limits_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
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
      auto risk_limits = RiskLimits{
          .label = "test"sv,  // XXX
          .account = {},
          .user = user.name,
          .limits = limits_,
          .session_id = last_session_id_,
          .seqno = last_seqno_,
      };
      log::debug("risk_limits={}"sv, risk_limits);
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_users(callback);
}

}  // namespace risk_manager
}  // namespace roq
