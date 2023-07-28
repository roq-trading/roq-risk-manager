/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <span>
#include <utility>

#include "roq/risk_manager/database/position.hpp"
#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Session {
  Session(Session &&) = default;
  Session(Session const &) = delete;

  virtual ~Session() = default;

  // get

  virtual void operator()(std::function<void(Trade const &)> const &) = 0;     // details
  virtual void operator()(std::function<void(Position const &)> const &) = 0;  // aggregate

  // put

  virtual void operator()(std::span<Trade const> const &) = 0;

 protected:
  Session() = default;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
