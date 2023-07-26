/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/third_party/sqlite/connection.hpp"

namespace roq {
namespace risk_manager {
namespace database {

struct Session final {
  static std::unique_ptr<Session> create();

  Session(Session &&) = default;
  Session(Session const &) = delete;

 protected:
  Session();

 private:
  std::unique_ptr<third_party::sqlite::Connection> connection_;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq
