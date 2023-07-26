/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

namespace roq {
namespace risk_manager {
namespace database {

struct Session {
  Session(Session &&) = default;
  Session(Session const &) = delete;

  virtual ~Session() = default;

 protected:
  Session() = default;
};

}  // namespace database
}  // namespace risk_manager
}  // namespace roq