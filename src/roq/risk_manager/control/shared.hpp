/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/risk_manager/settings.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Shared final {
  explicit Shared(Settings const &);

  Shared(Shared const &) = delete;
  Shared(Shared &&) = default;

  std::string_view const url_prefix;

  std::string encode_buffer;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
