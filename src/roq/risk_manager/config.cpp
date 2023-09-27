/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/config.hpp"

#include <toml++/toml.h>

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

// === CONSTANTS ===

namespace {
auto const EXCHANGE = "deribit"sv;  // XXX DEBUG
}

// === HELPERS ===

namespace {
void check_empty(auto &node) {
  if (!node.is_table())
    return;
  auto &table = *node.as_table();
  auto error = false;
  for (auto &[key, value] : table) {
    log::warn(R"(key="{}")"sv, static_cast<std::string_view>(key));
    error = true;
  }
  if (error)
    log::fatal("Unexpected"sv);
}

template <typename Callback>
bool find_and_remove(auto &node, std::string_view const &key, Callback callback) {
  if (!node.is_table()) {
    log::warn("Unexpected: node is not a table"sv);
    return false;
  }
  auto &table = *node.as_table();
  auto iter = table.find(key);
  if (iter == table.end())
    return false;
  callback((*iter).second);
  table.erase(iter);
  assert(table.find(key) == std::end(table));
  return true;
}

template <typename R>
R parse_symbols(auto &node) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  auto parse_helper = [&](auto &node) {
    using value_type = typename result_type::mapped_type::value_type;
    if (node.is_value()) {
      result[EXCHANGE].emplace(*node.template value<value_type>());
    } else if (node.is_array()) {
      auto &arr = *node.as_array();
      for (auto &node_2 : arr) {
        result[EXCHANGE].emplace(*node_2.template value<value_type>());
      }
    } else {
      log::fatal("Unexpected"sv);
    }
  };
  if (find_and_remove(node, "symbols"sv, parse_helper)) {
  } else {
    log::fatal(R"(Unexpected: did not find the "symbols" table)"sv);
  }
  return result;
}

template <typename R>
R parse_limits(auto &node, auto const &name) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  auto parse_helper = [&](auto &node) {
    if (node.is_table()) {
      auto &table = *node.as_table();
      for (auto &[key_1, value_1] : table) {
        std::string account{key_1};
        auto &tmp_1 = result[account];
        if (value_1.is_table()) {
          auto &table_1 = *value_1.as_table();
          for (auto &[key_2, value_2] : table_1) {
            std::string exchange{key_2};
            auto &tmp_2 = tmp_1[exchange];
            if (value_2.is_table()) {
              auto &table_2 = *value_2.as_table();
              for (auto &[key_3, value_3] : table_2) {
                std::string symbol{key_3};
                if (value_3.is_table()) {
                  auto &table_3 = *value_3.as_table();
                  risk::Limit limit;
                  find_and_remove(table_3, "long_position_limit"sv, [&](auto &value) {
                    limit.long_position_limit = *value.template value<double>();
                  });
                  find_and_remove(table_3, "short_position_limit"sv, [&](auto &value) {
                    limit.short_position_limit = *value.template value<double>();
                  });
                  find_and_remove(table_3, "long_risk_exposure_limit"sv, [&](auto &value) {
                    limit.long_risk_exposure_limit = *value.template value<double>();
                  });
                  find_and_remove(table_3, "short_risk_exposure_limit"sv, [&](auto &value) {
                    limit.short_risk_exposure_limit = *value.template value<double>();
                  });
                  find_and_remove(table_3, "allow_netting"sv, [&](auto &value) {
                    limit.allow_netting = *value.template value<bool>();
                  });
                  tmp_2.try_emplace(symbol, std::move(limit));
                  check_empty(value_3);
                } else {
                  log::fatal("Unexpected"sv);
                }
              }
            } else {
              log::fatal("Unexpected"sv);
            }
          }
        } else {
          log::fatal("Unexpected"sv);
        }
      }
    } else {
      log::fatal("Unexpected"sv);
    }
  };
  if (find_and_remove(node, name, parse_helper)) {
  } else {
    log::warn(R"(Did not find a "{}" table)"sv, name);
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Config Config::parse_file(std::string_view const &path) {
  auto root = toml::parse_file(path);
  return Config{root};
}

Config Config::parse_text(std::string_view const &text) {
  auto root = toml::parse(text);
  return Config{root};
}

Config::Config(auto &node)
    : symbols{parse_symbols<decltype(symbols)>(node)}, accounts{parse_limits<decltype(accounts)>(node, "accounts"sv)},
      users{parse_limits<decltype(accounts)>(node, "users"sv)} {
  check_empty(node);
  log::debug("config={}"sv, *this);
}

void Config::dispatch(Handler &handler) const {
  // accounts
  for (auto &[name, _] : accounts) {
    auto account = client::Account{
        .regex = name,
    };
    handler(account);
  }
  // symbols
  for (auto &[exchange, symbols_2] : symbols)
    for (auto &item : symbols_2) {
      auto symbol = client::Symbol{
          .regex = item,
          .exchange = exchange,
      };
      handler(symbol);
    }
  // currencies
  auto currency = client::Symbol{
      .regex = ".*",
      .exchange = {},
  };
  handler(currency);
}

}  // namespace risk_manager
}  // namespace roq
