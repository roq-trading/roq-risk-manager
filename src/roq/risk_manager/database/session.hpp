/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/risk_manager/database/callback.hpp"
#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Session {
  Session(Session &&) = default;
  Session(Session const &) = delete;

  virtual ~Session() = default;

  // get

  virtual void operator()(Callback<Trade> &) = 0;

  // put

  virtual void operator()(std::span<Trade const> const &) = 0;

 protected:
  Session() = default;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
