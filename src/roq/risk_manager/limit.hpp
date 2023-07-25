/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include <limits>

namespace roq {
namespace risk_manager {

struct Limit final {
  double long_limit = std::numeric_limits<double>::quiet_NaN();
  double short_limit = std::numeric_limits<double>::quiet_NaN();
};

}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::Limit> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::Limit const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(long_limit={}, )"
        R"(short_limit={})"
        R"(}})"_cf,
        value.long_limit,
        value.short_limit);
  }
};
