/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/database/session.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {
namespace database {

Session::Session() {
}

std::unique_ptr<Session> Session::create() {
  return nullptr;
}

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
