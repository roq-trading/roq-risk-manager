/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/risk_manager/database/session.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Factory final {
  static std::unique_ptr<Session> create(std::string_view const &type, std::string_view const &params);
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
