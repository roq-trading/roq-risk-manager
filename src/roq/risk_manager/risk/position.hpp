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

  double long_position() const { return long_position_; }
  double short_position() const { return short_position_; }

  double long_position_limit() const;
  double short_position_limit() const;

  double long_risk_exposure_limit() const;
  double short_risk_exposure_limit() const;

  template <typename Context>
  auto format_to(Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(allow_netting={}, )"
        R"(long_position_limit={}, )"
        R"(short_position_limit={}, )"
        R"(long_risk_exposure_limit={}, )"
        R"(short_risk_exposure_limit={}, )"
        R"(long_position={}, )"
        R"(short_position={}, )"
        R"(fills=[{}])"
        R"(}})"_cf,
        allow_netting,
        long_position_limit_,
        short_position_limit_,
        long_risk_exposure_limit_,
        short_risk_exposure_limit_,
        long_position_,
        short_position_,
        fmt::join(fills_, ", "sv));
  }

  bool const allow_netting;

 private:
  double const long_position_limit_;
  double const short_position_limit_;
  double const long_risk_exposure_limit_;
  double const short_risk_exposure_limit_;
  absl::flat_hash_set<std::string> fills_;  // history
  // DEBUG
  double quantity_ = {};
  double long_position_ = {};
  double short_position_ = {};
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
