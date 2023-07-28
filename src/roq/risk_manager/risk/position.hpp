/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <limits>

#include "roq/reference_data.hpp"
#include "roq/trade_update.hpp"

#include "roq/risk_manager/database/position.hpp"

#include "roq/risk_manager/risk/instrument.hpp"
#include "roq/risk_manager/risk/limit.hpp"

namespace roq {
namespace risk_manager {
namespace risk {

struct Position final {
  explicit Position(Limit const &);

  Position(Position &&) = default;
  Position(Position const &) = delete;

  void operator()(database::Position const &, Instrument const &);

  void operator()(ReferenceData const &, Instrument const &);
  void operator()(TradeUpdate const &, Instrument const &);

  double long_quantity() const { return long_quantity_; }
  double short_quantity() const { return short_quantity_; }

  double buy_limit() const;
  double sell_limit() const;

  template <typename Context>
  auto format_to(Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(long_limit={}, )"
        R"(short_limit={}, )"
        R"(long_quantity={}, )"
        R"(short_quantity={}, )"
        R"(fills=[{}])"
        R"(}})"_cf,
        long_limit_,
        short_limit_,
        long_quantity_,
        short_quantity_,
        fmt::join(fills_, ", "sv));
  }

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

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::risk::Position> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::risk::Position const &value, Context &context) const {
    return value.format_to(context);
  }
};
