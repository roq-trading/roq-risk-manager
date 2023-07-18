/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/reference_data.hpp"
#include "roq/trade_update.hpp"

#include "position.hpp"

namespace simple {

struct Shared;  // note! circular dependency

struct User final {
  User(std::string_view const &name, Shared &);

  User(User &&) = default;
  User(User const &) = delete;

  std::string const name;

  void operator()(roq::ReferenceData const &);
  void operator()(roq::TradeUpdate const &);

  template <typename Callback>
  void get_position(uint32_t instrument_id, Callback callback) {
    auto iter = positions_.find(instrument_id);
    if (iter != std::end(positions_))
      callback((*iter).second);
  }

 protected:
  template <typename Callback>
  void dispatch(auto &value, Callback);

 private:
  Shared &shared_;
  absl::flat_hash_map<uint32_t, Position> positions_;
};

}  // namespace simple
