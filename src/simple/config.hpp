/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <string>

namespace simple {

struct Config final {
  static Config parse_file(std::string_view const &);
  static Config parse_text(std::string_view const &);

  absl::flat_hash_set<std::string> const symbols;

 private:
  explicit Config(auto &node);
};

}  // namespace simple

template <>
struct fmt::formatter<simple::Config> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(simple::Config const &value, Context &context) const {
    using namespace std::literals;
    using namespace fmt::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbols=[{}])"
        R"(}})"_cf,
        fmt::join(value.symbols, ", "sv));
  }
};
