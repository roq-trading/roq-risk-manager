/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include "roq/args/parser.hpp"

#include "roq/client/flags/settings.hpp"

#include "roq/risk_manager/flags/flags.hpp"

namespace roq {
namespace risk_manager {

struct Settings final : public client::flags::Settings, public flags::Flags {
  explicit Settings(args::Parser const &);
};

}  // namespace risk_manager
}  // namespace roq
