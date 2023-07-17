/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/client.hpp"

#include "simple/account.hpp"
#include "simple/config.hpp"
#include "simple/instrument.hpp"
#include "simple/limit.hpp"

namespace simple {

struct Shared final {
  explicit Shared(Config const &);

  Instrument &get_instrument(std::string_view const &exchange, std::string_view const &symbol);

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

  Limit get_limit(
      std::string_view const &account, std::string_view const &exchange, std::string_view const &symbol) const;

 protected:
  uint32_t get_instrument_id(std::string_view const &exchange, std::string_view const &symbol);

 private:
  uint32_t next_instrument_id_ = {};
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, int32_t>> instrument_lookup_;
  absl::flat_hash_map<uint32_t, Instrument> instruments_;
  absl::flat_hash_map<std::string, Account> accounts_;  // note! can't make const
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Limit>>> const
      limits_;
};

}  // namespace simple
