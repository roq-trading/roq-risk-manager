/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <limits>

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

  auto long_quantity() const { return long_quantity_; }
  auto short_quantity() const { return short_quantity_; }

  auto buy_limit() const { return long_limit_ - long_quantity_; }
  auto sell_limit() const { return short_limit_ - short_quantity_; }

 private:
  double const long_limit_;
  double const short_limit_;
  absl::flat_hash_set<std::string> fills_;  // history
  // DEBUG
  double quantity_ = {};
  double long_quantity_ = {};
  double short_quantity_ = {};
  // TEST
  int64_t current_ = {};  // XXX TODO issues min_trade_vol changing over time
};

}  // namespace simple
