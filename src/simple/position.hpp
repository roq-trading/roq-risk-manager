/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include "roq/reference_data.hpp"
#include "roq/trade_update.hpp"

#include "simple/instrument.hpp"
#include "simple/limit.hpp"

namespace simple {

struct Position final {
  explicit Position(Limit const &);

  Position(Position &&) = default;
  Position(Position const &) = delete;

  void operator()(roq::ReferenceData const &, Instrument const &);
  void operator()(roq::TradeUpdate const &, Instrument const &);

  auto buy_limit() const {
    return long_limit_;  // XXX TODO
  }
  auto sell_limit() const {
    return short_limit_;  // XXX TODO
  }

 private:
  double const long_limit_;
  double const short_limit_;
  absl::flat_hash_set<std::string> fills_;  // history
  int64_t current_ = {};
  double test_ = {};  // DEBUG
};

}  // namespace simple
