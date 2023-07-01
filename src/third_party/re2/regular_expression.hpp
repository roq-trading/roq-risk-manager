/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <re2/re2.h>

#include <memory>
#include <string>
#include <string_view>

namespace third_party {
namespace re2 {

struct RegularExpression final {
  using value_type = ::re2::RE2;

  explicit RegularExpression(std::string_view const &pattern);

  RegularExpression(RegularExpression const &);

  RegularExpression(RegularExpression &&) = default;

  bool match(std::string_view const &text) const;

  std::pair<bool, std::string> extract(std::string_view const &text) const;

 private:
  std::unique_ptr<value_type> regex_;
};

}  // namespace re2
}  // namespace third_party
