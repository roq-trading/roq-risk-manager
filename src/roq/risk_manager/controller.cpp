/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/controller.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "roq/risk_manager/database/factory.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

// === IMPLEMENTATION ===

Controller::Controller(
    client::Dispatcher &dispatcher,
    Settings const &settings,
    Config const &config,
    roq::io::Context &context,
    size_t source_count)
    : dispatcher_{dispatcher}, context_{context},
      database_{database::Factory::create(settings.db_type, settings.db_params)},
      control_manager_{*this, settings, context_, shared_, *database_}, shared_{settings, config},
      state_(source_count) {
  load_positions();
}

// client::Handler

// note! timer is used to achieve batching of updates => gateway & clients can proxy until updates arrive
void Controller::operator()(Event<Timer> const &event) {
  context_.drain();
  control_manager_(event);
  for (size_t source = 0; source < std::size(state_); ++source) {
    auto const &state = state_[source];
    if (!state.ready)  // note! wait for "ready" (all accounts have then been downloaded)
      continue;
    publish_accounts(static_cast<uint8_t>(source));
    publish_users(static_cast<uint8_t>(source));
  }
}

void Controller::operator()(Event<Connected> const &) {
}

void Controller::operator()(Event<Disconnected> const &event) {
  auto &[message_info, disconnected] = event;
  state_[message_info.source] = {};
  log::warn("*** NOT READY *** (source={})"sv, message_info.source);
}

void Controller::operator()(Event<DownloadBegin> const &event) {
  auto &[message_info, download_begin] = event;
  (*this)(message_info);
  if (std::empty(download_begin.account))
    return;
  log::warn(
      R"(*** DOWNLOAD NOW IN PROGRESS *** (source={}, account="{}"))"sv, message_info.source, download_begin.account);
}

void Controller::operator()(Event<DownloadEnd> const &event) {
  auto &[message_info, download_end] = event;
  (*this)(message_info);
  if (std::empty(download_end.account))
    return;
  log::warn(R"(*** DOWNLOAD HAS COMPLETED *** (source={}, account="{}"))"sv, message_info.source, download_end.account);
  auto &state = state_[message_info.source];
  auto res = state.latch_by_account.emplace(download_end.account);
  if (res.second)
    shared_.publish_account(download_end.account);
}

void Controller::operator()(Event<Ready> const &event) {
  auto &[message_info, ready] = event;
  (*this)(message_info);
  auto &state = state_[message_info.source];
  if (state.ready)
    return;
  state.ready = true;
  log::warn("*** READY *** (source={})"sv, message_info.source);
  auto callback = [&](auto &user) { shared_.publish_user(user.name); };
  shared_.get_all_users(callback);
}

// XXX TODO also use MarketByPrice in case reference data not available...?
void Controller::operator()(Event<ReferenceData> const &event) {
  auto &reference_data = event.value;
  // log::debug("reference_data={}"sv, reference_data);
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
// however, for high volume throughput one might consider buffering and postpone persisting until the timer event fires
void Controller::operator()(Event<TradeUpdate> const &event) {
  log::info<1>("event={}"sv, event);
  (*this)(event.message_info);
  auto &trade_update = event.value;
  // note! we drop any trades prior to our last seen exchange time
  if (trade_update.create_time_utc <= last_exchange_time_utc_) {
    log::warn<1>("*** DROP *** ({} <= {})"sv, trade_update.create_time_utc, last_exchange_time_utc_);
    return;
  }
  log::debug("trade_update={}"sv, trade_update);
  // positions
  auto callback = [&](auto &item) { item(trade_update); };
  shared_.get_account(trade_update.account, callback);
  shared_.get_user(trade_update.user, callback);
  // database
  try {
    trades_buffer_.clear();
    for (auto &item : trade_update.fills) {
      auto trade = database::Trade{
          .user = trade_update.user,
          .strategy_id = trade_update.strategy_id,
          .account = trade_update.account,
          .exchange = trade_update.exchange,
          .symbol = trade_update.symbol,
          .side = trade_update.side,
          .quantity = item.quantity,
          .price = item.price,
          .exchange_time_utc = trade_update.create_time_utc,
          .external_account = trade_update.external_account,
          .external_order_id = trade_update.external_order_id,
          .external_trade_id = item.external_trade_id,
      };
      // subscribers
      control_manager_(trade);
      // for database
      trades_buffer_.emplace_back(std::move(trade));
    }
    (*database_)(trades_buffer_);
  } catch (...) {
    // XXX TODO more specific
  }
}

// XXX TODO perhaps useful to persist this into the database?
void Controller::operator()(Event<PositionUpdate> const &event) {
  log::info<1>("event={}"sv, event);
  auto &[message_info, position_update] = event;
  log::debug("position_update={}"sv, position_update);
  // database
  // cache
  auto &account = shared_.accounts_by_source[message_info.source][position_update.account];
  auto &position = account.positions[position_update.exchange][position_update.symbol];
  if (position(position_update)) {
    // XXX TODO notify subscribers
  }
}

// XXX TODO perhaps useful to persist this into the database?
void Controller::operator()(Event<FundsUpdate> const &event) {
  log::info<1>("event={}"sv, event);
  auto &[message_info, funds_update] = event;
  log::debug("funds_update={}"sv, funds_update);
  // database
  {
    auto funds = database::Funds{
        .account = funds_update.account,
        .currency = funds_update.currency,
        .balance = funds_update.balance,
        .hold = funds_update.hold,
        .exchange_time_utc = funds_update.exchange_time_utc,
        .external_account = funds_update.external_account,
    };
    (*database_)({&funds, 1});
  }
  // cache
  auto &account = shared_.accounts_by_source[message_info.source][funds_update.account];
  auto &funds = account.funds[funds_update.currency];
  if (funds(funds_update)) {
    // XXX TODO notify subscribers
  }
}

// control::Manager::Handler
// note! empty

// utilities

void Controller::operator()(MessageInfo const &message_info) {
  auto &state = state_[message_info.source];
  state.session_id = message_info.source_session_id;
  state.seqno = message_info.source_seqno;
}

void Controller::publish_accounts(uint8_t source) {
  auto const &state = state_[source];
  auto callback = [&](auto &account) {
    risk_limits_buffer_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_position = position.long_position(),
          .short_position = position.short_position(),
          .long_position_limit = position.long_position_limit(),
          .short_position_limit = position.short_position_limit(),
          .long_risk_exposure_limit = position.long_risk_exposure_limit(),
          .short_risk_exposure_limit = position.short_risk_exposure_limit(),
          .allow_netting = position.allow_netting,
      };
      risk_limits_buffer_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_account(account.name, callback);
    if (!std::empty(risk_limits_buffer_)) {
      auto risk_limits = RiskLimits{
          .user = {},
          .strategy_id = {},
          .account = account.name,
          .limits = risk_limits_buffer_,
          .session_id = state.session_id,
          .seqno = state.seqno,
      };
      log::debug("risk_limits={}"sv, risk_limits);
      dispatcher_.send(risk_limits, source);
    }
  };
  shared_.get_all_accounts(callback);
}

// XXX doesn't support multiple gateways
void Controller::publish_users(uint8_t source) {
  auto const &state = state_[source];
  auto callback = [&](auto &user) {
    risk_limits_buffer_.clear();
    auto callback = [&](auto &position, auto &instrument) {
      auto risk_limit = RiskLimit{
          .exchange = instrument.exchange,
          .symbol = instrument.symbol,
          .long_position = position.long_position(),
          .short_position = position.short_position(),
          .long_position_limit = position.long_position_limit(),
          .short_position_limit = position.short_position_limit(),
          .long_risk_exposure_limit = position.long_risk_exposure_limit(),
          .short_risk_exposure_limit = position.short_risk_exposure_limit(),
          .allow_netting = position.allow_netting,
      };
      risk_limits_buffer_.emplace_back(std::move(risk_limit));
    };
    shared_.get_publish_by_user(user.name, callback);  // XXX
    if (!std::empty(risk_limits_buffer_)) {
      auto risk_limits = RiskLimits{
          .user = user.name,
          .strategy_id = {},
          .account = {},
          .limits = risk_limits_buffer_,
          .session_id = state.session_id,
          .seqno = state.seqno,
      };
      log::debug("risk_limits={}"sv, risk_limits);
      dispatcher_.send(risk_limits, source);
    }
  };
  shared_.get_all_users(callback);
}

void Controller::load_positions() {
  auto dispatch = [&last_exchange_time_utc = last_exchange_time_utc_,
                   &shared = shared_](database::Position const &position) {
    auto callback = [&](auto &item) { item(position); };
    log::debug("position={}"sv, position);
    last_exchange_time_utc = std::max(last_exchange_time_utc, position.exchange_time_utc);
    if (!std::empty(position.user)) {
      assert(std::empty(position.account));
      shared.get_user(position.user, callback);
    }
    if (!std::empty(position.account)) {
      assert(std::empty(position.user));
      shared.get_account(position.account, callback);
    }
  };
  (*database_)(dispatch);
}

}  // namespace risk_manager
}  // namespace roq
