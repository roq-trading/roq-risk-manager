/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/account.hpp"

#include "simple/shared.hpp"

using namespace std::literals;

namespace simple {

Account::Account(std::string_view const &name, Shared &shared) : name{name}, shared_{shared} {
}

void Account::operator()(roq::ReferenceData const &reference_data) {
  dispatch(reference_data);
}

void Account::operator()(roq::TradeUpdate const &trade_update) {
  dispatch(trade_update);
}

void Account::dispatch(auto &value) {
  auto &instrument = shared_.get_instrument(value.exchange, value.symbol);
  auto iter = positions_.find(instrument.id);
  if (iter == std::end(positions_)) {
    auto limit = shared_.get_limit(name, value.exchange, value.symbol);
    iter = positions_.try_emplace(instrument.id, limit).first;
  }
  (*iter).second(value, *this, instrument);
}

}  // namespace simple
