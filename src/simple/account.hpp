/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/reference_data.hpp"
#include "roq/trade_update.hpp"

#include "position.hpp"

namespace simple {

struct Shared;  // note! circular dependency

struct Account final {
  Account(std::string_view const &name, Shared &);

  Account(Account &&) = default;
  Account(Account const &) = delete;

  std::string const name;

  void operator()(roq::ReferenceData const &);
  void operator()(roq::TradeUpdate const &);

 protected:
  void dispatch(auto &value);

 private:
  Shared &shared_;
  absl::flat_hash_map<uint32_t, Position> positions_;
};

}  // namespace simple
