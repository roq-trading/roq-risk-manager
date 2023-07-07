/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include "roq/trade_update.hpp"

namespace simple {

struct Shared;  // note! circular dependency

struct Account final {
  explicit Account(Shared &);

  Account(Account &&) = default;
  Account(Account const &) = delete;

  void operator()(roq::TradeUpdate const &);

 private:
  Shared &shared_;
  absl::flat_hash_set<std::string> fills_;
  absl::flat_hash_map<std::string, int64_t> positions_;
};

}  // namespace simple
