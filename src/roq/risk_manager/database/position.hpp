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

struct Position final {
  std::string_view user;
  uint32_t strategy_id = {};
  std::string_view account;
  std::string_view exchange;
  std::string_view symbol;
  double long_quantity = NaN;
  double short_quantity = NaN;
  std::chrono::nanoseconds create_time_utc = {};
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Position> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Position const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(user="{}", )"
        R"(strategy_id={}, )"
        R"(account="{}", )"
        R"(exchange="{}", )"
        R"(symbol="{}", )"
        R"(long_quantity={}, )"
        R"(short_quantity={}, )"
        R"(create_time_utc={})"
        R"(}})"_cf,
        value.user,
        value.strategy_id,
        value.account,
        value.exchange,
        value.symbol,
        value.long_quantity,
        value.short_quantity,
        value.create_time_utc);
  }
};
