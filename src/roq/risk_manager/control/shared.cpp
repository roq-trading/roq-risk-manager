/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/shared.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace control {

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings) : url_prefix{settings.control_url_prefix} {
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
