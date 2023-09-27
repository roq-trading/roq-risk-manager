/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/risk/position.hpp"

#include "roq/logging.hpp"

#include "roq/utils/common.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace risk {

Position::Position(Limit const &limit)
    : allow_netting{limit.allow_netting}, long_position_limit_{limit.long_position_limit},
      short_position_limit_{limit.short_position_limit}, long_risk_exposure_limit_{limit.long_risk_exposure_limit},
      short_risk_exposure_limit_{limit.short_risk_exposure_limit} {
}

void Position::operator()(database::Position const &position, Instrument const &) {
  long_position_ = position.long_quantity;
  short_position_ = position.short_quantity;
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
    long_position_ = std::max(quantity_, 0.0);
    short_position_ = std::max(-quantity_, 0.0);
    // TEST
    auto quantity = instrument.quantity_to_internal(item.quantity);
    if (!quantity) [[unlikely]] {
      log::warn("Probably something wrong... perhaps missing reference data?"sv);
    }
    current_ += sign * quantity;
  }
}

double Position::long_position_limit() const {
  // return std::max(0.0, long_position_limit_ - long_position_);
  return long_position_limit_;
}

double Position::short_position_limit() const {
  // return std::max(0.0, short_position_limit_ - short_position_);
  return short_position_limit_;
}

double Position::long_risk_exposure_limit() const {
  // return std::max(0.0, long_risk_exposure_limit_ - long_position_);
  return long_risk_exposure_limit_;
}

double Position::short_risk_exposure_limit() const {
  // return std::max(0.0, short_risk_exposure_limit_ - short_position_);
  return short_risk_exposure_limit_;
}

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq
