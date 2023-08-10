/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/risk/strategy.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace risk {

Strategy::Strategy(uint32_t strategy_id, Handler &handler) : strategy_id{strategy_id}, handler_{handler} {
}

void Strategy::operator()(database::Position const &position) {
  auto callback = []([[maybe_unused]] auto instrument_id) {};
  dispatch(position, callback);
}

void Strategy::operator()(ReferenceData const &reference_data) {
  auto callback = []([[maybe_unused]] auto instrument_id) {};
  dispatch(reference_data, callback);
}

void Strategy::operator()(TradeUpdate const &trade_update) {
  auto callback = [this](auto instrument_id) { handler_.publish_strategy(strategy_id, instrument_id); };
  dispatch(trade_update, callback);
}

template <typename Callback>
void Strategy::dispatch(auto &value, Callback callback) {
  auto &instrument = handler_.get_instrument(value.exchange, value.symbol);
  auto iter = positions_.find(instrument.id);
  if (iter == std::end(positions_)) {
    auto limit = handler_.get_limit_by_strategy(strategy_id, value.exchange, value.symbol);
    iter = positions_.try_emplace(instrument.id, limit).first;
  }
  auto &position = (*iter).second;
  position(value, instrument);
  callback(instrument.id);
  log::debug(
      R"(strategy_id={}, exchange="{}", symbol="{}", instrument={}, position={})"sv,
      strategy_id,
      value.exchange,
      value.symbol,
      instrument,
      position);
}

}  // namespace risk
}  // namespace risk_manager
}  // namespace roq
