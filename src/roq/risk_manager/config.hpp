/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <fmt/compile.h>
#include <fmt/format.h>

#include <string>
#include <string_view>

#include "roq/client/config.hpp"

#include "roq/risk_manager/limit.hpp"

namespace roq {
namespace risk_manager {

struct Config final : public client::Config {
  static Config parse_file(std::string_view const &);
  static Config parse_text(std::string_view const &);

  // exchange => symbol
  absl::flat_hash_map<std::string, absl::flat_hash_set<std::string>> const symbols;

  // account => exchange => symbol => limit
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Limit>>> const
      accounts;

  // user => exchange => symbol => limit
  absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Limit>>> const
      users;

 protected:
  explicit Config(auto &node);

  void dispatch(Handler &) const override;
};

}  // namespace risk_manager
}  // namespace roq
