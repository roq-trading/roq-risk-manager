/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>

#include <limits>

namespace roq {
namespace risk_manager {
namespace risk {

struct Limit final {
  double long_position_limit = std::numeric_limits<double>::quiet_NaN();
  double short_position_limit = std::numeric_limits<double>::quiet_NaN();
  double long_risk_exposure_limit = std::numeric_limits<double>::quiet_NaN();
  double short_risk_exposure_limit = std::numeric_limits<double>::quiet_NaN();
  bool allow_netting = {};
};

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::risk::Limit> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::risk::Limit const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(long_position_limit={}, )"
        R"(short_position_limit={}, )"
        R"(long_risk_exposure_limit={}, )"
        R"(short_risk_exposure_limit={}, )"
        R"(allow_netting={})"
        R"(}})"_cf,
        value.long_position_limit,
        value.short_position_limit,
        value.long_risk_exposure_limit,
        value.short_risk_exposure_limit,
        value.allow_netting);
  }
};
