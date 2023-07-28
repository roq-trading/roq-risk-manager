/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite/session.hpp"

#include "roq/logging.hpp"

#include "roq/third_party/sqlite/statement.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

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
      "  user TEXT, "
      "  account TEXT NOT NULL, "
      "  exchange TEXT NOT NULL, "
      "  symbol TEXT NOT NULL, "
      "  side TEXT NOT NULL, "
      "  quantity REAL NOT NULL, "
      "  price REAL NOT NULL, "
      "  create_time_utc INTEGER NOT NULL, "
      "  external_account TEXT, "
      "  external_order_id TEXT, "
      "  external_trade_id TEXT NOT NULL, "
      "  PRIMARY KEY ("
      "    account, "
      "    exchange, "
      "    symbol, "
      "    create_time_utc, "
      "    external_trade_id"
      "  )"
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
  create_index("user"sv);
  create_index("account"sv);
  create_index("exchange"sv);
  create_index("symbol"sv);
  create_index("side"sv);
  create_index("create_time_utc"sv, true);
}

auto create_table_trades_insert_statement(auto &connection, auto &table_name, auto &trade) {
  auto query = fmt::format(
      "INSERT OR REPLACE "
      "INTO {} "
      "VALUES ('{}','{}','{}','{}','{}',{},{},{},'{}','{}','{}')"sv,
      table_name,
      trade.user,
      trade.account,
      trade.exchange,
      trade.symbol,
      trade.side,
      trade.quantity,
      trade.price,
      trade.create_time_utc.count(),
      trade.external_account,
      trade.external_order_id,
      trade.external_trade_id);
  // log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}

auto create_positions_by_user_statement(auto &connection, auto &table_name) {
  auto query = fmt::format(
      "SELECT "
      "  t1.user, "
      "  '' AS account, "
      "  t1.exchange, "
      "  t1.symbol, "
      "  SUM(COALESCE(t1.quantity, 0)) AS long_quantity, "
      "  SUM(COALESCE(t2.quantity, 0)) AS short_quantity, "
      "  MAX("
      "    COALESCE(t1.create_time_utc, 0), "
      "    COALESCE(t2.create_time_utc, 0) "
      "  ) AS create_time_utc "
      "FROM ( "
      "  SELECT "
      "    user,"
      "    exchange,"
      "    symbol,"
      "    SUM(quantity) AS quantity, "
      "    MAX(create_time_utc) AS create_time_utc "
      "  FROM {} "
      "  WHERE side='BUY' "
      "  GROUP BY "
      "    user,"
      "    exchange,"
      "    symbol "
      ") t1 "
      "FULL OUTER JOIN ( "
      "  SELECT "
      "    user,"
      "    exchange,"
      "    symbol,"
      "    SUM(quantity) AS quantity, "
      "    MAX(create_time_utc) AS create_time_utc "
      "  FROM {} "
      "  WHERE "
      "    side='SELL'  "
      "  GROUP BY "
      "    user,"
      "    exchange,"
      "    symbol"
      ") t2 "
      "ON "
      "  t1.user=t2.user AND "
      "  t1.exchange=t2.exchange AND "
      "  t1.symbol=t2.symbol"sv,
      table_name,
      table_name);
  // log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}
auto create_positions_by_account_statement(auto &connection, auto &table_name) {
  auto query = fmt::format(
      "SELECT "
      "  '' AS user, "
      "  t1.account, "
      "  t1.exchange, "
      "  t1.symbol, "
      "  SUM(COALESCE(t1.quantity, 0)) AS long_quantity, "
      "  SUM(COALESCE(t2.quantity, 0)) AS short_quantity, "
      "  MAX("
      "    COALESCE(t1.create_time_utc, 0), "
      "    COALESCE(t2.create_time_utc, 0) "
      "  ) AS create_time_utc "
      "FROM ( "
      "  SELECT "
      "    account,"
      "    exchange,"
      "    symbol,"
      "    SUM(quantity) AS quantity, "
      "    MAX(create_time_utc) AS create_time_utc "
      "  FROM {} "
      "  WHERE side='BUY' "
      "  GROUP BY "
      "    account,"
      "    exchange,"
      "    symbol "
      ") t1 "
      "FULL OUTER JOIN ( "
      "  SELECT "
      "    account,"
      "    exchange,"
      "    symbol,"
      "    SUM(quantity) AS quantity, "
      "    MAX(create_time_utc) AS create_time_utc "
      "  FROM {} "
      "  WHERE "
      "    side='SELL'  "
      "  GROUP BY "
      "    account,"
      "    exchange,"
      "    symbol"
      ") t2 "
      "ON "
      "  t1.account=t2.account AND "
      "  t1.exchange=t2.exchange AND "
      "  t1.symbol=t2.symbol"sv,
      table_name,
      table_name);
  // log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params) : connection_{create_connection(params)} {
  create_table_trades(*connection_, TABLE_NAME_TRADES);
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
  auto dispatch = [&](auto &&statement) {
    while (statement.step()) {
      auto user = statement.template get<std::string>(0);
      auto account = statement.template get<std::string>(1);
      auto exchange = statement.template get<std::string>(2);
      auto symbol = statement.template get<std::string>(3);
      auto long_quantity = statement.template get<double>(4);
      auto short_quantity = statement.template get<double>(5);
      auto create_time_utc = statement.template get<int64_t>(6);
      auto position = Position{
          .user = user,
          .account = account,
          .exchange = exchange,
          .symbol = symbol,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .create_time_utc = std::chrono::nanoseconds{create_time_utc},
      };
      callback(position);
    }
  };
  dispatch(create_positions_by_user_statement(*connection_, TABLE_NAME_TRADES));
  dispatch(create_positions_by_account_statement(*connection_, TABLE_NAME_TRADES));
}

void Session::operator()(std::span<Trade const> const &trades) {
  for (auto &item : trades) {
    auto statement = create_table_trades_insert_statement(*connection_, TABLE_NAME_TRADES, item);
    statement.step();
  }
}

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
