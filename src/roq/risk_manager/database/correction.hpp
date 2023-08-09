/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <string_view>

#include "roq/api.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Correction final {
  std::string_view user;
  uint32_t strategy_id = {};
  std::string_view account;
  std::string_view exchange;
  std::string_view symbol;
  Side side = {};
  double quantity = NaN;
  double price = NaN;
  std::chrono::nanoseconds exchange_time_utc = {};  // note! missing means "now"
  // note! following might be useful for reconciliation purposes
  std::string_view reason;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Correction> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Correction const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(user="{}", )"
        R"(strategy_id={}, )"
        R"(account="{}", )"
        R"(exchange="{}", )"
        R"(symbol="{}", )"
        R"(side={}, )"
        R"(quantity={}, )"
        R"(price={}, )"
        R"(exchange_time_utc={}, )"
        R"(reason="{}")"
        R"(}})"_cf,
        value.user,
        value.strategy_id,
        value.account,
        value.exchange,
        value.symbol,
        value.side,
        value.quantity,
        value.price,
        value.exchange_time_utc,
        value.reason);
  }
};
