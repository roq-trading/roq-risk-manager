/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <string_view>

namespace roq {
namespace risk_manager {
namespace database {

struct Compress final {
  std::chrono::nanoseconds exchange_time_utc = {};  // note! missing means "all"
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::database::Compress> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::database::Compress const &value, Context &context) const {
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange_time_utc={})"
        R"(}})"_cf,
        value.exchange_time_utc);
  }
};
