/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/client.hpp"

#include "roq/risk_manager/account.hpp"
#include "roq/risk_manager/config.hpp"
#include "roq/risk_manager/instrument.hpp"
#include "roq/risk_manager/limit.hpp"
#include "roq/risk_manager/settings.hpp"
#include "roq/risk_manager/user.hpp"

#include "roq/risk_manager/database/session.hpp"

namespace roq {
namespace risk_manager {

struct Shared final {
  Shared(Settings const &, Config const &);

  Instrument &get_instrument(std::string_view const &exchange, std::string_view const &symbol);

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

  Limit get_limit_by_account(
      std::string_view const &account, std::string_view const &exchange, std::string_view const &symbol) const;

  void publish_account(std::string_view const &account);
  void publish_account(std::string_view const &account, uint32_t instrument_id) {
    publish_by_account_[account].emplace(instrument_id);
  }

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

  Limit get_limit_by_user(
      std::string_view const &user, std::string_view const &exchange, std::string_view const &symbol) const;

  void publish_user(std::string_view const &user);
  void publish_user(std::string_view const &user, uint32_t instrument_id) {
    publish_by_user_[user].emplace(instrument_id);
  }

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

 protected:
  uint32_t get_instrument_id(std::string_view const &exchange, std::string_view const &symbol);

 private:
  std::unique_ptr<database::Session> database_;
  uint32_t next_instrument_id_ = {};
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, int32_t>> instrument_lookup_;
  absl::flat_hash_map<uint32_t, Instrument> instruments_;
  absl::flat_hash_map<std::string, Account> accounts_;  // note! can't make const
  absl::flat_hash_map<std::string, User> users_;        // note! can't make const
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Limit>>> const
      limits_by_account_;
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Limit>>> const
      limits_by_user_;
  absl::flat_hash_map<std::string, absl::flat_hash_set<uint32_t>> publish_by_account_;
  absl::flat_hash_map<std::string, absl::flat_hash_set<uint32_t>> publish_by_user_;
};

}  // namespace risk_manager
}  // namespace roq
