/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

namespace roq {
namespace risk_manager {
namespace database {

template <typename T>
struct Callback {
  using value_type = T;
  virtual void operator()(T const &) = 0;
  virtual void finish() = 0;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
