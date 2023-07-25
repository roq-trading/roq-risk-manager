/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/position.hpp"

#include "roq/logging.hpp"

#include "roq/utils/common.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

Position::Position(Limit const &limit) : long_limit_{limit.long_limit}, short_limit_{limit.short_limit} {
}

void Position::operator()(ReferenceData const &, Instrument const &) {
  // XXX TODO we must re-scale when reference data changes
}

void Position::operator()(TradeUpdate const &trade_update, Instrument const &instrument) {
  auto sign = utils::sign(trade_update.side);
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
      log::warn("Probably something wrong... perhaps missing reference data?"sv);
    }
    current_ += sign * quantity;
  }
}

double Position::buy_limit() const {
  return std::max(0.0, long_limit_ - long_quantity_);
}

double Position::sell_limit() const {
  return std::max(0.0, short_limit_ - short_quantity_);
}

}  // namespace risk_manager
}  // namespace roq
