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

struct Funds final {
  std::string_view account;
  std::string_view currency;
  double balance = NaN;
  double hold = NaN;
  std::chrono::nanoseconds exchange_time_utc = {};
  // note! following might be useful for reconciliation purposes
  std::string_view external_account;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Funds> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Funds const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(account="{}", )"
        R"(currency="{}", )"
        R"(balance={}, )"
        R"(hold={}, )"
        R"(exchange_time_utc={}, )"
        R"(external_account="{}")"
        R"(}})"_cf,
        value.account,
        value.currency,
        value.balance,
        value.hold,
        value.exchange_time_utc,
        value.external_account);
  }
};
