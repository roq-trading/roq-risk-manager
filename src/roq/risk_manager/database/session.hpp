/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Session {
  Session(Session &&) = default;
  Session(Session const &) = delete;

  virtual ~Session() = default;

  virtual void put(Trade const &) = 0;

 protected:
  Session() = default;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
