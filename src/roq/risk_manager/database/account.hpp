/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <chrono>
#include <string_view>

namespace roq {
namespace risk_manager {
namespace database {

struct Account final {
  std::string_view name;
  std::chrono::nanoseconds exchange_time_utc_min = {};
  std::chrono::nanoseconds exchange_time_utc_max = {};
  uint64_t trade_count = {};
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Account> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Account const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(account="{}", )"
        R"(exchange_time_utc_min={}, )"
        R"(exchange_time_utc_max={}, )"
        R"(trade_count={})"
        R"(}})"_cf,
        value.name,
        value.exchange_time_utc_min,
        value.exchange_time_utc_max,
        value.trade_count);
  }
};
