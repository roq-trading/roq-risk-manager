/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/config.hpp"

namespace simple {

Config::Config(Settings const &settings) : settings_{settings} {
}

void Config::dispatch(Handler &handler) const {
  // accounts
  auto account = roq::client::Account{
      .regex = settings_.accounts,
  };
  handler(account);
  // symbols
  auto symbol = roq::client::Symbol{
      .regex = settings_.symbols,
      .exchange = settings_.exchange,
  };
  handler(symbol);
}

}  // namespace simple
