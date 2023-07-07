/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/decimals.hpp"
#include "roq/reference_data.hpp"

namespace simple {

struct Instrument final {
  Instrument() = default;

  Instrument(Instrument &&) = default;
  Instrument(Instrument const &) = delete;

  void operator()(roq::ReferenceData const &);

  int64_t quantity_as_integer(double quantity) const;

 private:
  roq::Decimals quantity_decimals_;
};

}  // namespace simple
