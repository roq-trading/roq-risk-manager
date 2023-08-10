/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/shared.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

// === HELPERS ===

template <typename R>
auto create_config(auto &config, auto &shared) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[name, _] : config)
    result.try_emplace(name, name, shared);
  return result;
}

template <typename R>
auto create_limits(auto &limits) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &[account, value_1] : limits) {
    auto &tmp_1 = result[account];
    for (auto &[exchange, value_2] : value_1) {
      auto &tmp_2 = tmp_1[exchange];
      for (auto &[symbol, limit] : value_2)
        tmp_2.try_emplace(symbol, limit);
    }
  }
  return result;
}

// === IMPLEMENTATION ===

Shared::Shared(Settings const &, Config const &config)
    : accounts_{create_config<decltype(accounts_)>(config.accounts, *this)},
      users_{create_config<decltype(users_)>(config.users, *this)},
      strategies_{create_config<decltype(strategies_)>(config.strategies, *this)},
      limits_by_account_{create_limits<decltype(limits_by_account_)>(config.accounts)},
      limits_by_user_{create_limits<decltype(limits_by_user_)>(config.users)},
      limits_by_strategy_{create_limits<decltype(limits_by_strategy_)>(config.strategies)} {
}

uint32_t Shared::get_instrument_id(std::string_view const &exchange, std::string_view const &symbol) {
  assert(!std::empty(symbol));
  auto &result = instrument_lookup_[exchange][symbol];
  if (!result)
    result = ++next_instrument_id_;
  return result;
}

risk::Instrument &Shared::get_instrument(std::string_view const &exchange, std::string_view const &symbol) {
  auto instrument_id = get_instrument_id(exchange, symbol);
  auto iter = instruments_.find(instrument_id);
  if (iter == std::end(instruments_))
    iter = instruments_.try_emplace(instrument_id, instrument_id, exchange, symbol).first;
  return (*iter).second;
}

// accounts

// note! lookup is rare -- no need to optimize
risk::Limit Shared::get_limit_by_account(
    std::string_view const &account, std::string_view const &exchange, std::string_view const &symbol) const {
  auto iter_1 = limits_by_account_.find(account);
  if (iter_1 == std::end(limits_by_account_))
    return {};
  auto &tmp_1 = (*iter_1).second;
  auto iter_2 = tmp_1.find(exchange);
  if (iter_2 == std::end(tmp_1))
    return {};
  auto &tmp_2 = (*iter_2).second;
  auto iter_3 = tmp_2.find(symbol);
  if (iter_3 == std::end(tmp_2))
    return {};
  return (*iter_3).second;
}

void Shared::publish_account(std::string_view const &account) {
  auto &tmp = publish_by_account_[account];
  for (auto &[instrument_id, _] : instruments_)
    tmp.emplace(instrument_id);
}

// users

// note! lookup is rare -- no need to optimize
risk::Limit Shared::get_limit_by_user(
    std::string_view const &user, std::string_view const &exchange, std::string_view const &symbol) const {
  auto iter_1 = limits_by_user_.find(user);
  if (iter_1 == std::end(limits_by_user_))
    return {};
  auto &tmp_1 = (*iter_1).second;
  auto iter_2 = tmp_1.find(exchange);
  if (iter_2 == std::end(tmp_1))
    return {};
  auto &tmp_2 = (*iter_2).second;
  auto iter_3 = tmp_2.find(symbol);
  if (iter_3 == std::end(tmp_2))
    return {};
  return (*iter_3).second;
}

void Shared::publish_user(std::string_view const &user) {
  auto &tmp = publish_by_user_[user];
  for (auto &[instrument_id, _] : instruments_)
    tmp.emplace(instrument_id);
}

// strategies

// note! lookup is rare -- no need to optimize
risk::Limit Shared::get_limit_by_strategy(
    uint32_t strategy_id, std::string_view const &exchange, std::string_view const &symbol) const {
  auto iter_1 = limits_by_strategy_.find(strategy_id);
  if (iter_1 == std::end(limits_by_strategy_))
    return {};
  auto &tmp_1 = (*iter_1).second;
  auto iter_2 = tmp_1.find(exchange);
  if (iter_2 == std::end(tmp_1))
    return {};
  auto &tmp_2 = (*iter_2).second;
  auto iter_3 = tmp_2.find(symbol);
  if (iter_3 == std::end(tmp_2))
    return {};
  return (*iter_3).second;
}

void Shared::publish_strategy(uint32_t strategy_id) {
  auto &tmp = publish_by_strategy_[strategy_id];
  for (auto &[instrument_id, _] : instruments_)
    tmp.emplace(instrument_id);
}

}  // namespace risk_manager
}  // namespace roq
