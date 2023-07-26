/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite.hpp"

#include "roq/logging.hpp"

#include "roq/third_party/sqlite/statement.hpp"

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

void create_table_trades(auto &connection, auto &table_name) {
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

auto create_table_trades_insert_statement(auto &connection, auto &table_name, auto &trade) {
  auto query = fmt::format(
      "INSERT OR REPLACE "
      "INTO {} "
      "VALUES ('{}','{}','{}','{}',{},{},{},{},'{}','{}','{}','{}')"sv,
      table_name,
      trade.account,
      trade.exchange,
      trade.symbol,
      trade.side,
      trade.quantity,
      trade.price,
      trade.create_time_utc.count(),
      trade.update_time_utc.count(),
      trade.user,
      trade.external_account,
      trade.external_order_id,
      trade.external_trade_id);
  log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}
}  // namespace

// === IMPLEMENTATION ===

SQLite::SQLite(std::string_view const &params) : connection_{create_connection(params)} {
  create_table_trades(*connection_, TABLE_NAME_TRADES);
}

void SQLite::operator()(std::span<Trade const> const &trades) {
  for (auto &item : trades) {
    auto statement = create_table_trades_insert_statement(*connection_, TABLE_NAME_TRADES, item);
    statement.step();
  }
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
