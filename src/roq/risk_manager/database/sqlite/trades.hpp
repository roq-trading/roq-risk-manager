/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/third_party/sqlite/connection.hpp"

#include "roq/risk_manager/database/account.hpp"
#include "roq/risk_manager/database/correction.hpp"
#include "roq/risk_manager/database/position.hpp"
#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

struct Trades final {
  // create

  static void create(third_party::sqlite::Connection &);

  // query

  static void select(third_party::sqlite::Connection &, std::function<void(Account const &)> const &);
  static void select(third_party::sqlite::Connection &, std::function<void(Position const &)> const &);
  static void select(
      third_party::sqlite::Connection &,
      std::function<void(Trade const &)> const &,
      std::string_view const &account,
      std::chrono::nanoseconds start_time);

  // insert

  static void insert(third_party::sqlite::Connection &, std::span<Trade const> const &);
  static void insert(third_party::sqlite::Connection &, std::span<Correction const> const &);

  // maintenance

  static void compress(third_party::sqlite::Connection &, std::chrono::nanoseconds exchange_time_utc);
};

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
