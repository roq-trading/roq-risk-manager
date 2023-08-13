/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/client.hpp"

#include "roq/cache/funds.hpp"
#include "roq/cache/position.hpp"

#include "roq/risk_manager/config.hpp"
#include "roq/risk_manager/settings.hpp"

#include "roq/risk_manager/risk/account.hpp"
#include "roq/risk_manager/risk/instrument.hpp"
#include "roq/risk_manager/risk/limit.hpp"
#include "roq/risk_manager/risk/strategy.hpp"
#include "roq/risk_manager/risk/user.hpp"

namespace roq {
namespace risk_manager {

struct Shared final : public risk::Account::Handler, public risk::User::Handler, public risk::Strategy::Handler {
  Shared(Settings const &, Config const &);

  Shared(Shared const &) = delete;
  Shared(Shared &&) = default;

  risk::Instrument &get_instrument(std::string_view const &exchange, std::string_view const &symbol) override;

  // accounts

  template <typename Callback>
  bool get_account(std::string_view const &account, Callback callback) {
    auto iter = accounts_.find(account);
    if (iter != std::end(accounts_)) {
      callback((*iter).second);
      return true;
    }
    return false;
  }

  template <typename Callback>
  void get_all_accounts(Callback callback) {
    for (auto &[_, account] : accounts_)
      callback(account);
  }

  void publish_account(std::string_view const &account);

  template <typename Callback>
  bool get_publish_by_account(std::string_view const &account, Callback callback) {
    auto iter_1 = publish_by_account_.find(account);
    if (iter_1 == std::end(publish_by_account_))
      return false;
    for (auto instrument_id : (*iter_1).second) {
      auto iter_2 = instruments_.find(instrument_id);
      if (iter_2 == std::end(instruments_))
        continue;  // XXX should never happen
      auto &instrument = (*iter_2).second;
      auto iter_3 = accounts_.find(account);
      if (iter_3 == std::end(accounts_))
        continue;  // XXX should never happen
      auto callback_2 = [&](auto &position) { callback(position, instrument); };
      auto &account = (*iter_3).second;
      account.get_position(instrument_id, callback_2);
    }
    (*iter_1).second.clear();
    return true;
  }

  // users

  template <typename Callback>
  bool get_user(std::string_view const &user, Callback callback) {
    auto iter = users_.find(user);
    if (iter != std::end(users_)) {
      callback((*iter).second);
      return true;
    }
    return false;
  }

  template <typename Callback>
  void get_all_users(Callback callback) {
    for (auto &[_, user] : users_)
      callback(user);
  }

  void publish_user(std::string_view const &user);

  template <typename Callback>
  bool get_publish_by_user(std::string_view const &user, Callback callback) {
    auto iter_1 = publish_by_user_.find(user);
    if (iter_1 == std::end(publish_by_user_))
      return false;
    for (auto instrument_id : (*iter_1).second) {
      auto iter_2 = instruments_.find(instrument_id);
      if (iter_2 == std::end(instruments_))
        continue;  // XXX should never happen
      auto &instrument = (*iter_2).second;
      auto iter_3 = users_.find(user);
      if (iter_3 == std::end(users_))
        continue;  // XXX should never happen
      auto callback_2 = [&](auto &position) { callback(position, instrument); };
      auto &user = (*iter_3).second;
      user.get_position(instrument_id, callback_2);
    }
    (*iter_1).second.clear();
    return true;
  }

  // strategies

  template <typename Callback>
  bool get_strategy(uint32_t strategy_id, Callback callback) {
    auto iter = strategies_.find(strategy_id);
    if (iter != std::end(strategies_)) {
      callback((*iter).second);
      return true;
    }
    return false;
  }

  template <typename Callback>
  void get_all_strategys(Callback callback) {
    for (auto &[_, strategy] : strategies_)
      callback(strategy);
  }

  void publish_strategy(uint32_t strategy_id);

  template <typename Callback>
  bool get_publish_by_strategy(uint32_t strategy_id, Callback callback) {
    auto iter_1 = publish_by_strategy_.find(strategy_id);
    if (iter_1 == std::end(publish_by_strategy_))
      return false;
    for (auto instrument_id : (*iter_1).second) {
      auto iter_2 = instruments_.find(instrument_id);
      if (iter_2 == std::end(instruments_))
        continue;  // XXX should never happen
      auto &instrument = (*iter_2).second;
      auto iter_3 = strategies_.find(strategy_id);
      if (iter_3 == std::end(strategies_))
        continue;  // XXX should never happen
      auto callback_2 = [&](auto &position) { callback(position, instrument); };
      auto &strategy = (*iter_3).second;
      strategy.get_position(instrument_id, callback_2);
    }
    (*iter_1).second.clear();
    return true;
  }

 protected:
  uint32_t get_instrument_id(std::string_view const &exchange, std::string_view const &symbol);

  // accounts

  risk::Limit get_limit_by_account(
      std::string_view const &account, std::string_view const &exchange, std::string_view const &symbol) const override;

  void publish_account(std::string_view const &account, uint32_t instrument_id) override {
    publish_by_account_[account].emplace(instrument_id);
  }

  // users

  risk::Limit get_limit_by_user(
      std::string_view const &user, std::string_view const &exchange, std::string_view const &symbol) const override;

  void publish_user(std::string_view const &user, uint32_t instrument_id) override {
    publish_by_user_[user].emplace(instrument_id);
  }

  // strategies

  risk::Limit get_limit_by_strategy(
      uint32_t strategy_id, std::string_view const &exchange, std::string_view const &symbol) const override;

  void publish_strategy(uint32_t strategy_id, uint32_t instrument_id) override {
    publish_by_strategy_[strategy_id].emplace(instrument_id);
  }

 private:
  uint32_t next_instrument_id_ = {};
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, int32_t>> instrument_lookup_;
  absl::flat_hash_map<uint32_t, risk::Instrument> instruments_;
  absl::flat_hash_map<std::string, risk::Account> accounts_;  // note! can't make const
  absl::flat_hash_map<std::string, risk::User> users_;        // note! can't make const
  absl::flat_hash_map<uint32_t, risk::Strategy> strategies_;  // note! can't make const
  absl::flat_hash_map<
      std::string,
      absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const limits_by_account_;
  absl::flat_hash_map<
      std::string,
      absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const limits_by_user_;
  absl::flat_hash_map<uint32_t, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const
      limits_by_strategy_;
  absl::flat_hash_map<std::string, absl::flat_hash_set<uint32_t>> publish_by_account_;
  absl::flat_hash_map<std::string, absl::flat_hash_set<uint32_t>> publish_by_user_;
  absl::flat_hash_map<uint32_t, absl::flat_hash_set<uint32_t>> publish_by_strategy_;

 public:
  struct Account final {
    // exchange -> symbol -> position
    absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, cache::Position>> positions;
    // currency -> funds
    absl::flat_hash_map<std::string, cache::Funds> funds;
  };
  // source -> account -> account
  absl::flat_hash_map<uint8_t, absl::flat_hash_map<std::string, Account>> accounts_by_source;
};

}  // namespace risk_manager
}  // namespace roq
