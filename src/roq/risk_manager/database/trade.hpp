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

// note! replace std::string_view with std::string if you need to buffer trade updates
struct Trade final {
  std::string_view user;
  uint32_t strategy_id = {};
  std::string_view account;
  std::string_view exchange;
  std::string_view symbol;
  Side side = {};
  double quantity = NaN;
  double price = NaN;
  std::chrono::nanoseconds exchange_time_utc = {};
  // note! following might be useful for reconciliation purposes
  std::string_view external_account;
  std::string_view external_order_id;
  std::string_view external_trade_id;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Trade> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Trade const &value, Context &context) const {
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
        R"(external_account="{}", )"
        R"(external_order_id="{}", )"
        R"(external_trade_id="{}")"
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
        value.external_account,
        value.external_order_id,
        value.external_trade_id);
  }
};
