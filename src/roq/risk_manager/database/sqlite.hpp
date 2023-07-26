/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/third_party/sqlite/connection.hpp"

#include "roq/risk_manager/database/session.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct SQLite final : public Session {
  explicit SQLite(std::string_view const &params);

 protected:
  void operator()(std::span<Trade const> const &) override;

 private:
  std::unique_ptr<third_party::sqlite::Connection> connection_;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
