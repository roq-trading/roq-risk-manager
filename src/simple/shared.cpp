/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/shared.hpp"

using namespace std::literals;

namespace simple {

// XXX TODO also support exchange
Instrument &Shared::get_instrument(std::string_view const &exchange, std::string_view const &symbol) {
  assert(!std::empty(symbol));
  auto iter = instruments_.find(symbol);
  if (iter == std::end(instruments_))
    iter = instruments_.try_emplace(symbol).first;
  return (*iter).second;
}

Account &Shared::get_account(std::string_view const &account) {
  assert(!std::empty(account));
  auto iter = accounts_.find(account);
  if (iter == std::end(accounts_))
    iter = accounts_.try_emplace(account, *this).first;
  return (*iter).second;
}

}  // namespace simple
