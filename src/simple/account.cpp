/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/account.hpp"

#include <cassert>

#include "simple/shared.hpp"

using namespace std::literals;

namespace simple {

Account::Account(Shared &shared) : shared_{shared} {
}

void Account::operator()(roq::TradeUpdate const &trade_update) {
  auto &instrument = shared_.get_instrument(trade_update.exchange, trade_update.symbol);
  for (auto &item : trade_update.fills) {
    assert(!std::empty(item.external_trade_id));
    auto res = fills_.emplace(item.external_trade_id);
    if (!res.second)  // note! avoid double-counting
      continue;
    auto quantity = instrument.quantity_as_integer(item.quantity);
    positions_[trade_update.symbol] += quantity;
  }
}

}  // namespace simple
