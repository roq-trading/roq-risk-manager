/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/position.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace simple {

Position::Position(Limit const &limit) : long_limit_{limit.long_limit}, short_limit_{limit.short_limit} {
}

void Position::operator()(roq::ReferenceData const &, Instrument const &) {
  // XXX TODO we must re-scale when reference data changes
}

void Position::operator()(roq::TradeUpdate const &trade_update, Instrument const &instrument) {
  for (auto &item : trade_update.fills) {
    assert(!std::empty(item.external_trade_id));
    auto res = fills_.emplace(item.external_trade_id);
    if (!res.second)  // note! don't double-count
      continue;
    auto quantity = instrument.quantity_to_internal(item.quantity);
    if (!quantity) [[unlikely]] {
      roq::log::warn("Probably something wrong... perhaps missing reference data?"sv);
    }
    current_ += quantity;
    test_ += item.quantity;
  }
}

}  // namespace simple
