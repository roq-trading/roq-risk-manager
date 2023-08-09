/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/third_party/sqlite/connection.hpp"

#include "roq/risk_manager/database/funds.hpp"

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

struct Funds final {
  // create

  static void create(third_party::sqlite::Connection &);

  // query

  static void select(
      third_party::sqlite::Connection &,
      std::function<void(database::Funds const &)> const &,
      std::string_view const &account,
      std::string_view const &currency);

  // insert

  static void insert(third_party::sqlite::Connection &, std::span<database::Funds const> const &);

  // maintenance

  static void compress(third_party::sqlite::Connection &, std::chrono::nanoseconds exchange_time_utc);
};

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
