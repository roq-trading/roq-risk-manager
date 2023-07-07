/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/config.hpp"

#include "simple/flags/flags.hpp"

namespace simple {

using flags::Flags;

void Config::dispatch(Handler &handler) const {
  // accounts
  auto account = roq::client::Account{
      .regex = Flags::accounts(),
  };
  handler(account);
  // symbols
  auto symbol = roq::client::Symbol{
      .regex = Flags::symbols(),
      .exchange = Flags::exchange(),
  };
  handler(symbol);
}

}  // namespace simple
