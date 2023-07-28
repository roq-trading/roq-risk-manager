/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/controller.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "roq/risk_manager/database/factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

// === CONSTANTS ===

namespace {
auto const RISK_LIMITS_LABEL = "risk"sv;
}

// === IMPLEMENTATION ===

Controller::Controller(
    client::Dispatcher &dispatcher, Settings const &settings, Config const &config, roq::io::Context &context)
    : dispatcher_{dispatcher}, context_{context},
      database_{database::Factory::create(settings.db_type, settings.db_params)}, control_manager_{settings, context_},
      shared_{settings, config} {
  load_trades();
  load_positions();
}

// client::Handler

// note! timer is used to achieve batching of updates => gateway & clients can proxy until updates arrive
void Controller::operator()(Event<Timer> const &event) {
  context_.drain();
  control_manager_(event);
  if (!ready_)  // note! wait for "ready" (all accounts have then been downloaded)
    return;
  publish_accounts();
  publish_users();
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &) {
  last_session_id_ = {};
  last_seqno_ = {};
  ready_ = {};
  latch_by_account_.clear();
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
  auto res = latch_by_account_.emplace(download_end.account);
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

// note!
// we are currently persisting trade updates synchronously
// this should not be an issue for low volume throughput
// however, for high volume throughput one might consider buffering and postpone
// persisting until the timer event fires
void Controller::operator()(Event<TradeUpdate> const &event) {
  log::info("event={}"sv, event);
  auto &[message_info, trade_update] = event;
  last_session_id_ = message_info.source_session_id;
  last_seqno_ = message_info.source_seqno;
  auto callback = [&](auto &item) { item(trade_update); };
  shared_.get_account(trade_update.account, callback);
  shared_.get_user(trade_update.user, callback);
  try {
    trades_buffer_.clear();
    for (auto &item : trade_update.fills) {
      auto trade = database::Trade{
          .user = trade_update.user,
          .account = trade_update.account,
          .exchange = trade_update.exchange,
          .symbol = trade_update.symbol,
          .side = trade_update.side,
          .quantity = item.quantity,
          .price = item.price,
          .create_time_utc = trade_update.create_time_utc,
          .update_time_utc = trade_update.update_time_utc,
          .external_account = trade_update.external_account,
          .external_order_id = trade_update.external_order_id,
          .external_trade_id = item.external_trade_id,
      };
      trades_buffer_.emplace_back(std::move(trade));
    }
    (*database_)(trades_buffer_);
  } catch (...) {
    // XXX TODO more specific
  }
}

// utilities

void Controller::publish_accounts() {
  auto callback = [&](auto &account) {
    risk_limits_buffer_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_quantity = position.long_quantity(),
          .short_quantity = position.short_quantity(),
          .buy_limit = position.buy_limit(),
          .sell_limit = position.sell_limit(),
      };
      risk_limits_buffer_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_account(account.name, callback);
    if (!std::empty(risk_limits_buffer_)) {
      auto risk_limits = RiskLimits{
          .label = RISK_LIMITS_LABEL,
          .account = account.name,
          .user = {},
          .limits = risk_limits_buffer_,
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
    risk_limits_buffer_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_quantity = position.long_quantity(),
          .short_quantity = position.short_quantity(),
          .buy_limit = position.buy_limit(),
          .sell_limit = position.sell_limit(),
      };
      risk_limits_buffer_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_user(user.name, callback);
    if (!std::empty(risk_limits_buffer_)) {
      auto risk_limits = RiskLimits{
          .label = RISK_LIMITS_LABEL,
          .account = {},
          .user = user.name,
          .limits = risk_limits_buffer_,
          .session_id = last_session_id_,
          .seqno = last_seqno_,
      };
      log::debug("risk_limits={}"sv, risk_limits);
      dispatcher_.send(risk_limits, 0);  // XXX
    }
  };
  shared_.get_all_users(callback);
}

void Controller::load_trades() {
  trades_buffer_.clear();
  auto callback = [&buffer = trades_buffer_](database::Trade const &trade) {
    log::debug("trade={}"sv, trade);
    buffer.emplace_back(trade);
  };
  (*database_)(callback);
}

void Controller::load_positions() {
  auto dispatch = [&shared = shared_](database::Position const &position) {
    auto callback = [&](auto &item) { item(position); };
    log::debug("position={}"sv, position);
    if (!std::empty(position.account))
      shared.get_account(position.account, callback);
    if (!std::empty(position.user))
      shared.get_user(position.user, callback);
  };
  (*database_)(dispatch);
}

}  // namespace risk_manager
}  // namespace roq
