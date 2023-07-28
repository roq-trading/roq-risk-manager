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
      "  update_time_utc INTEGER NOT NULL, "
      "  external_account TEXT, "
      "  external_order_id TEXT, "
      "  external_trade_id TEXT NOT NULL, "
      "  PRIMARY KEY ("
      "    account, "
      "    exchange, "
      "    symbol, "
      "    update_time_utc, "
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

auto create_table_trades_select_statement(auto &connection, auto &table_name) {
  auto query = fmt::format(
      "SELECT "
      "  user, "
      "  account, "
      "  exchange, "
      "  symbol, "
      "  side, "
      "  quantity, "
      "  price, "
      "  create_time_utc, "
      "  update_time_utc, "
      "  external_account,"
      "  external_order_id, "
      "  external_trade_id "
      "FROM {} "
      "ORDER BY "
      "  user, "
      "  account, "
      "  exchange, "
      "  symbol"sv,
      table_name);
  log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}

auto create_table_trades_insert_statement(auto &connection, auto &table_name, auto &trade) {
  auto query = fmt::format(
      "INSERT OR REPLACE "
      "INTO {} "
      "VALUES ('{}','{}','{}','{}','{}',{},{},{},{},'{}','{}','{}')"sv,
      table_name,
      trade.user,
      trade.account,
      trade.exchange,
      trade.symbol,
      trade.side,
      trade.quantity,
      trade.price,
      trade.create_time_utc.count(),
      trade.update_time_utc.count(),
      trade.external_account,
      trade.external_order_id,
      trade.external_trade_id);
  log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}

auto create_positions_select_statement(auto &connection, auto &table_name) {
  auto query = fmt::format(
      "SELECT "
      "  '' AS user, "
      "  account, "
      "  exchange, "
      "  symbol, "
      "  side, "
      "  sum(quantity), "
      "  max(update_time_utc) "
      "FROM {} "
      "GROUP BY "
      "  account, "
      "  exchange, "
      "  symbol, "
      "  side "
      "UNION "
      "SELECT "
      "  user, "
      "  '' AS account, "
      "  exchange, "
      "  symbol, "
      "  side, "
      "  sum(quantity), "
      "  max(update_time_utc) "
      "FROM {} "
      "GROUP BY "
      "  user, "
      "  exchange, "
      "  symbol, "
      "  side"
      ""sv,
      table_name,
      table_name);
  log::debug(R"(query="{}")"sv, query);
  return third_party::sqlite::Statement{connection, query};
}
}  // namespace

// === IMPLEMENTATION ===

Session::Session(std::string_view const &params) : connection_{create_connection(params)} {
  create_table_trades(*connection_, TABLE_NAME_TRADES);
}

void Session::operator()(std::function<void(Trade const &)> const &callback) {
  auto statement = create_table_trades_select_statement(*connection_, TABLE_NAME_TRADES);
  while (statement.step()) {
    auto user = statement.get<std::string>(0);
    auto account = statement.get<std::string>(1);
    auto exchange = statement.get<std::string>(2);
    auto symbol = statement.get<std::string>(3);
    auto side = statement.get<std::string>(4);
    auto quantity = statement.get<double>(5);
    auto price = statement.get<double>(6);
    auto create_time_utc = statement.get<int64_t>(7);
    auto update_time_utc = statement.get<int64_t>(8);
    auto external_account = statement.get<std::string>(9);
    auto external_order_id = statement.get<std::string>(10);
    auto external_trade_id = statement.get<std::string>(11);
    auto trade = Trade{
        .user = user,
        .account = account,
        .exchange = exchange,
        .symbol = symbol,
        .side = magic_enum::enum_cast<Side>(side).value(),
        .quantity = quantity,
        .price = price,
        .create_time_utc = std::chrono::nanoseconds{create_time_utc},
        .update_time_utc = std::chrono::nanoseconds{update_time_utc},
        .external_account = external_account,
        .external_order_id = external_order_id,
        .external_trade_id = external_trade_id,
    };
    callback(trade);
  }
}

void Session::operator()(std::function<void(Position const &)> const &callback) {
  auto statement = create_positions_select_statement(*connection_, TABLE_NAME_TRADES);
  while (statement.step()) {
    auto user = statement.get<std::string>(0);
    auto account = statement.get<std::string>(1);
    auto exchange = statement.get<std::string>(2);
    auto symbol = statement.get<std::string>(3);
    auto side = statement.get<std::string>(4);
    auto quantity = statement.get<double>(5);
    auto update_time_utc = statement.get<int64_t>(6);
    auto position = Position{
        .user = user,
        .account = account,
        .exchange = exchange,
        .symbol = symbol,
        .side = magic_enum::enum_cast<Side>(side).value(),
        .quantity = quantity,
        .update_time_utc = std::chrono::nanoseconds{update_time_utc},
    };
    callback(position);
  }
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
