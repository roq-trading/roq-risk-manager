/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/risk_manager/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

Settings::Settings(args::Parser const &args) : client::flags::Settings{args}, flags::Flags{flags::Flags::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace risk_manager
}  // namespace roq
