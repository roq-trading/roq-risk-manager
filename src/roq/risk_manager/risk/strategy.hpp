/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include "roq/reference_data.hpp"
#include "roq/trade_update.hpp"

#include "roq/risk_manager/database/position.hpp"

#include "roq/risk_manager/risk/instrument.hpp"
#include "roq/risk_manager/risk/position.hpp"

namespace roq {
namespace risk_manager {
namespace risk {

struct Strategy final {
  struct Handler {
    virtual void publish_strategy(uint32_t strategy_id, uint32_t instrument_id) = 0;
    virtual Instrument &get_instrument(std::string_view const &exchange, std::string_view const &symbol) = 0;
    virtual Limit get_limit_by_strategy(
        uint32_t strategy_id, std::string_view const &exchange, std::string_view const &symbol) const = 0;
  };

  Strategy(uint32_t strategy_id, Handler &);

  Strategy(Strategy &&) = default;
  Strategy(Strategy const &) = delete;

  uint32_t const strategy_id;

  void operator()(database::Position const &);

  void operator()(ReferenceData const &);
  void operator()(TradeUpdate const &);

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
  Handler &handler_;
  absl::flat_hash_map<uint32_t, Position> positions_;
};

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq
