/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <limits>

namespace simple {

struct Limit final {
  double long_limit = std::numeric_limits<double>::quiet_NaN();
  double short_limit = std::numeric_limits<double>::quiet_NaN();
};

}  // namespace simple
