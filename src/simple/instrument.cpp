/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/instrument.hpp"

#include "roq/client.hpp"

using namespace std::literals;

namespace simple {

Instrument::Instrument(uint32_t id, std::string_view const &exchange, std::string_view const &symbol)
    : id{id}, exchange{exchange}, symbol{symbol} {
}

bool Instrument::operator()(roq::ReferenceData const &reference_data) {
  if (std::isnan(reference_data.min_trade_vol))
    return false;
  min_trade_vol_ = reference_data.min_trade_vol;
  quantity_decimals_ = roq::client::Number::increment_to_decimals(min_trade_vol_);
  return true;
}

int64_t Instrument::quantity_to_internal(double quantity) const {
  auto result = roq::client::Quantity::to_internal(quantity, min_trade_vol_, quantity_decimals_);
  return static_cast<int64_t>(result);
}

}  // namespace simple
