/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include <limits>

#include "roq/api.hpp"

namespace roq {
namespace risk_manager {
namespace risk {

struct Instrument final {
  Instrument(uint32_t id, std::string_view const &exchange, std::string_view const &symbol);

  Instrument(Instrument &&) = default;
  Instrument(Instrument const &) = delete;

  uint32_t const id;
  Exchange const exchange;
  Symbol const symbol;

  bool operator()(ReferenceData const &);

  int64_t quantity_to_internal(double quantity) const;

  template <typename Context>
  auto format_to(Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(min_trade_vol={}, )"
        R"(quantity_decimals={})"
        R"(}})"_cf,
        min_trade_vol_,
        quantity_decimals_);
  }

 private:
  double min_trade_vol_ = std::numeric_limits<double>::quiet_NaN();
  Decimals quantity_decimals_;
};

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::risk::Instrument> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::risk::Instrument const &value, Context &context) const {
    return value.format_to(context);
  }
};
