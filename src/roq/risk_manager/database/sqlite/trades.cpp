/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite/trades.hpp"

#include "roq/logging.hpp"

#include "roq/third_party/sqlite/statement.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

// === CONSTANTS ===

namespace {
auto const TABLE_NAME = "trades"sv;
}

// === HELPERS ===

namespace {
auto select_positions_by_user(auto &connection) {
  auto query = fmt::format(
      "SELECT "
      "  t1.user, "
      "  0 AS strategy_id, "
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
      TABLE_NAME,
      TABLE_NAME);
  // log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}

auto select_positions_by_account(auto &connection) {
  auto query = fmt::format(
      "SELECT "
      "  '' AS user, "
      "  0 AS strategy_id, "
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
      TABLE_NAME,
      TABLE_NAME);
  // log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}
}  // namespace

// === IMPLEMENTATION ===

// XXX TODO maybe exchange + external_trade_id is enough for primary key?
void Trades::create(third_party::sqlite::Connection &connection) {
  log::info(R"(Creating table "{}")"sv, TABLE_NAME);
  auto query = fmt::format(
      "CREATE TABLE IF NOT EXISTS {} ("
      "  user TEXT, "
      "  strategy_id INTEGER, "
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
      "    user, "
      "    strategy_id, "
      "    account, "
      "    exchange, "
      "    symbol, "
      "    create_time_utc, "
      "    external_trade_id"
      "  )"
      ")"sv,
      TABLE_NAME);
  connection.exec(query);
  auto create_index = [&](auto const &index_name, bool sorted = false) {
    std::string query;
    fmt::format_to(
        std::back_inserter(query),
        "CREATE INDEX IF NOT EXISTS idx_{}_{} ON {}({}"sv,
        TABLE_NAME,
        index_name,
        TABLE_NAME,
        index_name);
    if (sorted)
      fmt::format_to(std::back_inserter(query), " ASC"sv);
    fmt::format_to(std::back_inserter(query), ")"sv);
    connection.exec(query);
  };
  create_index("user"sv);
  create_index("strategy_id"sv);
  create_index("account"sv);
  create_index("exchange"sv);
  create_index("symbol"sv);
  create_index("side"sv);
  create_index("create_time_utc"sv, true);
}

void Trades::insert(third_party::sqlite::Connection &connection, std::span<Trade const> const &trades) {
  // XXX TODO use prepared statement
  auto insert_or_replace = [&](auto &trade) {
    auto query = fmt::format(
        "INSERT OR REPLACE "
        "INTO {} "
        "VALUES ('{}','{}','{}','{}','{}',{},{},{},'{}','{}','{}')"sv,
        TABLE_NAME,
        trade.user,
        trade.strategy_id,
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
    auto statement = third_party::sqlite::Statement{connection, query};
    statement.step();
  };
  for (auto &item : trades)
    insert_or_replace(item);
}

void Trades::select(
    third_party::sqlite::Connection &connection, std::function<void(Position const &)> const &callback) {
  auto dispatch = [&](auto &&statement) {
    while (statement.step()) {
      auto user = statement.template get<std::string>(0);
      auto strategy_id = statement.template get<uint32_t>(1);
      auto account = statement.template get<std::string>(2);
      auto exchange = statement.template get<std::string>(3);
      auto symbol = statement.template get<std::string>(4);
      auto long_quantity = statement.template get<double>(5);
      auto short_quantity = statement.template get<double>(6);
      auto create_time_utc = statement.template get<int64_t>(7);
      auto position = Position{
          .user = user,
          .strategy_id = strategy_id,
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
  dispatch(select_positions_by_user(connection));
  dispatch(select_positions_by_account(connection));
}

void Trades::select(
    third_party::sqlite::Connection &connection,
    std::function<void(Trade const &)> const &callback,
    std::string_view const &account,
    std::chrono::nanoseconds start_time) {
  auto query =
      "SELECT "
      "  user, "
      "  strategy_id, "
      "  account, "
      "  exchange, "
      "  symbol, "
      "  side, "
      "  quantity, "
      "  price, "
      "  create_time_utc, "
      "  external_account, "
      "  external_order_id, "
      "  external_trade_id "
      "FROM trades"sv;
  // log::debug(R"(query="{}")"sv, query);
  auto statement = third_party::sqlite::Statement{connection, query};
  while (statement.step()) {
    auto user = statement.template get<std::string>(0);
    auto strategy_id = statement.template get<uint32_t>(1);
    auto account = statement.template get<std::string>(2);
    auto exchange = statement.template get<std::string>(3);
    auto symbol = statement.template get<std::string>(4);
    auto side = statement.template get<std::string>(5);
    auto quantity = statement.template get<double>(6);
    auto price = statement.template get<double>(7);
    auto create_time_utc = statement.template get<int64_t>(8);
    auto external_account = statement.template get<std::string>(9);
    auto external_order_id = statement.template get<std::string>(10);
    auto external_trade_id = statement.template get<std::string>(11);
    auto trade = Trade{
        .user = user,
        .strategy_id = strategy_id,
        .account = account,
        .exchange = exchange,
        .symbol = symbol,
        .side = magic_enum::enum_cast<Side>(side).value(),  // XXX TODO exception handling
        .quantity = quantity,
        .price = price,
        .create_time_utc = std::chrono::nanoseconds{create_time_utc},
        .external_account = external_account,
        .external_order_id = external_order_id,
        .external_trade_id = external_trade_id,
    };
    callback(trade);
  }
}

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
