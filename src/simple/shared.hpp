/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/client.hpp"

#include "simple/account.hpp"
#include "simple/instrument.hpp"

namespace simple {

struct Shared final {
  Instrument &get_instrument(std::string_view const &exchange, std::string_view const &symbol);
  Account &get_account(std::string_view const &account);

 private:
  absl::flat_hash_map<std::string, Instrument> instruments_;
  absl::flat_hash_map<std::string, Account> accounts_;
};

}  // namespace simple
