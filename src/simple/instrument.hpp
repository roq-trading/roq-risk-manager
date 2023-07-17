/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <limits>

#include "roq/decimals.hpp"
#include "roq/reference_data.hpp"

namespace simple {

struct Instrument final {
  Instrument(uint32_t id);

  Instrument(Instrument &&) = default;
  Instrument(Instrument const &) = delete;

  uint32_t const id;

  bool operator()(roq::ReferenceData const &);

  int64_t quantity_to_internal(double quantity) const;

 private:
  double min_trade_vol_ = std::numeric_limits<double>::quiet_NaN();
  roq::Decimals quantity_decimals_;
};

}  // namespace simple
