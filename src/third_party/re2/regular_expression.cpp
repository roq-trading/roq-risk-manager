/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "third_party/re2/regular_expression.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace third_party {
namespace re2 {

RegularExpression::RegularExpression(std::string_view const &pattern) : regex_{std::make_unique<value_type>(pattern)} {
  if (!(*regex_).ok())
    throw roq::RuntimeError{R"(Invalid regex="{}")"sv, pattern};
}

RegularExpression::RegularExpression(RegularExpression const &rhs) : RegularExpression{(*rhs.regex_).pattern()} {
}

bool RegularExpression::match(std::string_view const &text) const {
  return value_type::FullMatch(text, *regex_);
}

std::pair<bool, std::string> RegularExpression::extract(std::string_view const &text) const {
  std::pair<bool, std::string> result;
  result.first = value_type::FullMatch(text, *regex_, &result.second);
  return result;
}

}  // namespace re2
}  // namespace third_party
