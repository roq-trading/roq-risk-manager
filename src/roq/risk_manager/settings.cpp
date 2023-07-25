/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/settings.hpp"

namespace roq {
namespace risk_manager {

Settings::Settings(args::Parser const &args) : client::flags::Settings{args}, flags::Flags{flags::Flags::create()} {
}

}  // namespace risk_manager
}  // namespace roq
