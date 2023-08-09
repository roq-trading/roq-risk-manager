/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <functional>
#include <span>
#include <utility>

#include "roq/risk_manager/database/account.hpp"
#include "roq/risk_manager/database/compress.hpp"
#include "roq/risk_manager/database/correction.hpp"
#include "roq/risk_manager/database/funds.hpp"
#include "roq/risk_manager/database/position.hpp"
#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Session {
  Session(Session &&) = default;
  Session(Session const &) = delete;

  virtual ~Session() = default;

  // query

  virtual void operator()(std::function<void(Account const &)> const &) = 0;
  virtual void operator()(std::function<void(Position const &)> const &) = 0;
  virtual void operator()(
      std::function<void(Trade const &)> const &,
      std::string_view const &account,
      std::chrono::nanoseconds start_time) = 0;
  virtual void operator()(
      std::function<void(Funds const &)> const &,
      std::string_view const &account,
      std::string_view const &currency) = 0;

  // insert

  virtual void operator()(std::span<Trade const> const &) = 0;
  virtual void operator()(std::span<Correction const> const &) = 0;
  virtual void operator()(std::span<Funds const> const &) = 0;

  // maintenance
  virtual void operator()(Compress const &) = 0;

 protected:
  Session() = default;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
