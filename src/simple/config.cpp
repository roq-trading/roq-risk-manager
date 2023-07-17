/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/config.hpp"

#include <toml++/toml.h>

#include "roq/logging.hpp"

using namespace std::literals;

namespace simple {

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
    roq::log::warn(R"(key="{}")"sv, static_cast<std::string_view>(key));
    error = true;
  }
  if (error)
    roq::log::fatal("Unexpected"sv);
}

template <typename Callback>
bool find_and_remove(auto &node, std::string_view const &key, Callback callback) {
  if (!node.is_table()) {
    roq::log::warn("Unexpected: node is not a table"sv);
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
R parse_accounts(auto &node) {
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
                  Limit limit;
                  find_and_remove(table_3, "long_limit"sv, [&](auto &value) {
                    limit.long_limit = *value.template value<double>();
                  });
                  find_and_remove(table_3, "short_limit"sv, [&](auto &value) {
                    limit.short_limit = *value.template value<double>();
                  });
                  tmp_2.try_emplace(symbol, std::move(limit));
                  check_empty(value_3);
                } else {
                  roq::log::fatal("Unexpected"sv);
                }
              }
            } else {
              roq::log::fatal("Unexpected"sv);
            }
          }
        } else {
          roq::log::fatal("Unexpected"sv);
        }
      }
    } else {
      roq::log::fatal("Unexpected"sv);
    }
  };
  if (find_and_remove(node, "accounts"sv, parse_helper)) {
  } else {
    roq::log::fatal(R"(Unexpected: did not find the "accounts" table)"sv);
  }
  return result;
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
      roq::log::fatal("Unexpected"sv);
    }
  };
  if (find_and_remove(node, "symbols"sv, parse_helper)) {
  } else {
    roq::log::fatal(R"(Unexpected: did not find the "symbols" table)"sv);
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
    : accounts{parse_accounts<decltype(accounts)>(node)}, symbols{parse_symbols<decltype(symbols)>(node)} {
  check_empty(node);
}

void Config::dispatch(Handler &handler) const {
  // accounts
  for (auto &[name, _] : accounts) {
    auto account = roq::client::Account{
        .regex = name,
    };
    handler(account);
  }
  // symbols
  for (auto &[exchange, symbols_2] : symbols)
    for (auto &item : symbols_2) {
      auto symbol = roq::client::Symbol{
          .regex = item,
          .exchange = exchange,
      };
      handler(symbol);
    }
}

}  // namespace simple
