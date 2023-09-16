/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/sqlite/funds.hpp"

#include "roq/logging.hpp"

#include "roq/third_party/sqlite/statement.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

// === CONSTANTS ===

namespace {
auto const TABLE_NAME = "funds"sv;
}

// === IMPLEMENTATION ===

// create

// XXX TODO maybe exchange + external_trade_id is enough for primary key?
void Funds::create(third_party::sqlite::Connection &connection) {
  log::info(R"(Creating table "{}")"sv, TABLE_NAME);
  auto query = fmt::format(
      "CREATE TABLE IF NOT EXISTS {} ("
      "  account TEXT NOT NULL, "
      "  currency TEXT NOT NULL, "
      "  balance REAL NOT NULL, "
      "  hold REAL NOT NULL, "
      "  exchange_time_utc INTEGER NOT NULL, "
      "  external_account TEXT, "
      "  PRIMARY KEY ("
      "    account, "
      "    currency, "
      "    exchange_time_utc "
      "  )"
      ")"sv,
      TABLE_NAME);
  log::debug(R"(query="{}")"sv, query);
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
  create_index("account"sv);
  create_index("currency"sv);
  create_index("exchange_time_utc"sv, true);
}

// select

void Funds::select(
    third_party::sqlite::Connection &connection,
    std::function<void(database::Funds const &)> const &callback,
    std::string_view const &account,
    std::string_view const &currency) {
  auto query = fmt::format(
      "SELECT "
      "  t1.account, "
      "  t1.currency, "
      "  t1.balance, "
      "  t1.hold, "
      "  t1.exchange_time_utc, "
      "  t1.external_account "
      "FROM {} AS t1 "
      "JOIN ("
      "  SELECT "
      "    account, "
      "    currency, "
      "    MAX(exchange_time_utc) AS exchange_time_utc "
      "  FROM {} "sv,
      TABLE_NAME,
      TABLE_NAME);
  if (!(std::empty(account) && std::empty(currency))) {
    fmt::format_to(std::back_inserter(query), "WHERE "sv);
    if (!std::empty(account))
      fmt::format_to(std::back_inserter(query), "account='{}' "sv, account);
    if (!std::empty(currency)) {
      if (!std::empty(account))
        fmt::format_to(std::back_inserter(query), "AND "sv);
      fmt::format_to(std::back_inserter(query), "currency='{}' "sv, currency);
    }
  }
  fmt::format_to(
      std::back_inserter(query),
      "  GROUP BY "
      "    account, "
      "    currency "
      ") t2 "
      "ON "
      "  t1.account = t2.account AND "
      "  t1.currency = t2.currency AND "
      "  t1.exchange_time_utc = t2.exchange_time_utc "
      "ORDER BY "
      "  t1.account, "
      "  t1.currency"sv);
  log::debug(R"(query="{}")"sv, query);
  auto statement = third_party::sqlite::Statement{connection, query};
  while (statement.step()) {
    auto account = statement.template get<std::string>(0);
    auto currency = statement.template get<std::string>(1);
    auto balance = statement.template get<double>(2);
    auto hold = statement.template get<double>(3);
    auto exchange_time_utc = statement.template get<int64_t>(4);
    auto external_account = statement.template get<std::string>(5);
    auto funds = database::Funds{
        .account = account,
        .currency = currency,
        .balance = balance,
        .hold = hold,
        .exchange_time_utc = std::chrono::nanoseconds{exchange_time_utc},
        .external_account = external_account,
    };
    callback(funds);
  }
}

// insert

void Funds::insert(third_party::sqlite::Connection &connection, std::span<database::Funds const> const &funds) {
  auto now = clock::get_realtime();
  // XXX TODO use prepared statement
  auto insert_or_replace = [&](auto &item) {
    auto exchange_time_utc = item.exchange_time_utc.count() ? item.exchange_time_utc : now;
    auto query = fmt::format(
        "INSERT OR REPLACE "
        "INTO {} "
        "VALUES ('{}','{}',{},{},{},'{}')"sv,
        TABLE_NAME,
        item.account,
        item.currency,
        item.balance,
        std::isnan(item.hold) ? 0.0 : item.hold,
        exchange_time_utc.count(),
        item.external_account);
    log::debug(R"(query="{}")"sv, query);
    auto statement = third_party::sqlite::Statement{connection, query};
    statement.step();
  };
  for (auto &item : funds)
    insert_or_replace(item);
}

// maintenance

void Funds::compress(third_party::sqlite::Connection &, [[maybe_unused]] std::chrono::nanoseconds exchange_time_utc) {
  // XXX TODO accumulate all "trades" prior to exchange_time_utc and create positions (remember group-by)
}

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
