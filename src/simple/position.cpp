/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/position.hpp"

#include "roq/logging.hpp"

#include "roq/utils/common.hpp"

using namespace std::literals;

namespace simple {

Position::Position(Limit const &limit) : long_limit_{limit.long_limit}, short_limit_{limit.short_limit} {
}

void Position::operator()(roq::ReferenceData const &, Instrument const &) {
  // XXX TODO we must re-scale when reference data changes
}

void Position::operator()(roq::TradeUpdate const &trade_update, Instrument const &instrument) {
  auto sign = roq::utils::sign(trade_update.side);
  for (auto &item : trade_update.fills) {
    // note! avoid double-counting
    assert(!std::empty(item.external_trade_id));
    auto res = fills_.emplace(item.external_trade_id);
    if (!res.second)
      continue;
    // note! this could be more complex, e.g. omnibus accounting
    quantity_ += sign * item.quantity;
    long_quantity_ = std::max(quantity_, 0.0);
    short_quantity_ = std::max(-quantity_, 0.0);
    // TEST
    auto quantity = instrument.quantity_to_internal(item.quantity);
    if (!quantity) [[unlikely]] {
      roq::log::warn("Probably something wrong... perhaps missing reference data?"sv);
    }
    current_ += sign * quantity;
  }
}

}  // namespace simple
