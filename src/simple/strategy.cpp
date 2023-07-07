/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/strategy.hpp"

#include <cassert>

#include "roq/logging.hpp"

#include "tools/simple.hpp"

using namespace std::literals;

namespace simple {

Strategy::Strategy(roq::client::Dispatcher &dispatcher, Shared &shared) : dispatcher_{dispatcher}, shared_{shared} {
}

void Strategy::operator()(roq::Event<roq::Timer> const &) {
}

void Strategy::operator()(roq::Event<roq::Connected> const &) {
}

void Strategy::operator()(roq::Event<roq::Disconnected> const &) {
}

void Strategy::operator()(roq::Event<roq::DownloadBegin> const &event) {
  auto &[message_info, download_begin] = event;
  roq::log::warn(R"(*** DOWNLOAD NOW IN PROGRESS *** (account="{}"))"sv, download_begin.account);
}

void Strategy::operator()(roq::Event<roq::DownloadEnd> const &event) {
  auto &[message_info, download_end] = event;
  roq::log::warn<0>(
      R"(*** DOWNLOAD HAS COMPLETED *** (account="{}", max_order_id={}))"sv,
      download_end.account,
      download_end.max_order_id);
}

void Strategy::operator()(roq::Event<roq::GatewayStatus> const &) {
}

void Strategy::operator()(roq::Event<roq::ReferenceData> const &event) {
  auto &[message_info, reference_data] = event;
  auto &instrument = shared_.get_instrument(reference_data.exchange, reference_data.symbol);
  instrument(reference_data);
}

void Strategy::operator()(roq::Event<roq::OrderUpdate> const &event) {
  roq::log::info("event={}"sv, event);
}

void Strategy::operator()(roq::Event<roq::TradeUpdate> const &event) {
  roq::log::info("event={}"sv, event);
  auto &[message_info, trade_update] = event;
  auto &account = shared_.get_account(trade_update.account);
  account(trade_update);
}

}  // namespace simple
