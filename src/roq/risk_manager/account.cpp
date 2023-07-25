/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/account.hpp"

#include "roq/logging.hpp"

#include "roq/risk_manager/shared.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

Account::Account(std::string_view const &name, Shared &shared) : name{name}, shared_{shared} {
}

void Account::operator()(ReferenceData const &reference_data) {
  auto callback = []([[maybe_unused]] auto instrument_id) {};
  dispatch(reference_data, callback);
}

void Account::operator()(TradeUpdate const &trade_update) {
  auto callback = [this](auto instrument_id) { shared_.publish_account(name, instrument_id); };
  dispatch(trade_update, callback);
}

template <typename Callback>
void Account::dispatch(auto &value, Callback callback) {
  auto &instrument = shared_.get_instrument(value.exchange, value.symbol);
  auto iter = positions_.find(instrument.id);
  if (iter == std::end(positions_)) {
    auto limit = shared_.get_limit_by_account(name, value.exchange, value.symbol);
    iter = positions_.try_emplace(instrument.id, limit).first;
  }
  auto &position = (*iter).second;
  position(value, instrument);
  callback(instrument.id);
  log::debug(
      R"(account="{}, exchange="{}", symbol="{}", instrument={}, position={})"sv,
      name,
      value.exchange,
      value.symbol,
      instrument,
      position);
}

}  // namespace risk_manager
}  // namespace roq
