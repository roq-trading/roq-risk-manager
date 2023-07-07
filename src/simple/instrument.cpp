/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/instrument.hpp"

#include "roq/client.hpp"

using namespace std::literals;

namespace simple {

void Instrument::operator()(roq::ReferenceData const &reference_data) {
  if (!std::isnan(reference_data.min_trade_vol))
    quantity_decimals_ = roq::client::Number::increment_to_decimals(reference_data.min_trade_vol);
}

int64_t Instrument::quantity_as_integer(double quantity) const {
  return static_cast<int64_t>(quantity);  // XXX TODO use decimals (need API)
}

}  // namespace simple
