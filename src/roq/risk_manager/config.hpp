/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <string>
#include <string_view>

#include "roq/client/config.hpp"

#include "roq/risk_manager/risk/limit.hpp"

namespace roq {
namespace risk_manager {

struct Config final : public client::Config {
  static Config parse_file(std::string_view const &);
  static Config parse_text(std::string_view const &);

  // exchange => symbol
  absl::flat_hash_map<std::string, absl::flat_hash_set<std::string>> const symbols;

  // account => exchange => symbol => limit
  absl::flat_hash_map<
      std::string,
      absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const accounts;

  // user => exchange => symbol => limit
  absl::flat_hash_map<
      std::string,
      absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const users;

  // strategy => exchange => symbol => limit
  absl::flat_hash_map<uint32_t, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, risk::Limit>>> const
      strategies;

  template <typename Context>
  auto format_to(Context &context) const {
    using namespace fmt::literals;
    using namespace std::literals;
    // XXX TODO formatting of nested maps
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbols=[], )"
        R"(accounts=[], )"
        R"(users=[])"
        R"(}})"_cf);
    // fmt::join(symbols, ", "sv));
  }

 protected:
  explicit Config(auto &node);

  void dispatch(Handler &) const override;
};

}  // namespace risk_manager
}  // namespace roq

template <>
struct fmt::formatter<roq::risk_manager::Config> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::risk_manager::Config const &value, Context &context) const {
    return value.format_to(context);
  }
};
