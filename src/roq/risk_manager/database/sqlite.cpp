/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {

// === CONSTANTS ===

namespace {
auto const TABLE_NAME_TRADES = "trades"sv;
}

// === HELPERS ===

namespace {
auto create_connection(auto &params) {
  return std::make_unique<third_party::sqlite::Connection>(params);
}

void create_trades(auto &connection, auto &table_name) {
  log::info(R"(Creating table "{}")"sv, table_name);
  auto query = fmt::format(
      "CREATE TABLE IF NOT EXISTS {} ("
      "account TEXT NOT NULL, "
      "exchange TEXT NOT NULL, "
      "symbol TEXT NOT NULL, "
      "side TEXT NOT NULL, "
      "quantity REAL NOT NULL, "
      "price REAL NOT NULL, "
      "create_time_utc INTEGER NOT NULL, "
      "update_time_utc INTEGER NOT NULL, "
      "user TEXT, "
      "external_account TEXT, "
      "external_order_id TEXT, "
      "external_trade_id TEXT NOT NULL, "
      "PRIMARY KEY (account, exchange, symbol, update_time_utc, external_trade_id)"
      ")"sv,
      table_name);
  connection.exec(query);
  auto create_index = [&](auto const &index_name, bool sorted = false) {
    std::string query;
    fmt::format_to(
        std::back_inserter(query),
        "CREATE INDEX IF NOT EXISTS idx_{}_{} ON {}({}"sv,
        table_name,
        index_name,
        table_name,
        index_name);
    if (sorted)
      fmt::format_to(std::back_inserter(query), " ASC"sv);
    fmt::format_to(std::back_inserter(query), ")"sv);
    connection.exec(query);
  };
  create_index("account"sv);
  create_index("exchange"sv);
  create_index("symbol"sv);
  create_index("create_time_utc"sv, true);
  create_index("user"sv);
}
}  // namespace

// === IMPLEMENTATION ===

SQLite::SQLite(std::string_view const &params) : connection_{create_connection(params)} {
  create_trades(*connection_, TABLE_NAME_TRADES);
}

void SQLite::put(Trade const &) {
  log::fatal("Not implemented"sv);
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
