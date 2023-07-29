/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/third_party/sqlite/connection.hpp"

#include "roq/risk_manager/database/position.hpp"
#include "roq/risk_manager/database/trade.hpp"

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

struct Trades final {
  static void create(third_party::sqlite::Connection &);
  static void insert(third_party::sqlite::Connection &, std::span<Trade const> const &);
  static void select(third_party::sqlite::Connection &, std::function<void(Position const &)> const &);
};

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
