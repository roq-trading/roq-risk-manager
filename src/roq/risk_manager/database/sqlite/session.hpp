/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include "roq/third_party/sqlite/connection.hpp"

#include "roq/risk_manager/database/session.hpp"

namespace roq {
namespace risk_manager {
namespace database {
namespace sqlite {

struct Session final : public database::Session {
  explicit Session(std::string_view const &params);

 protected:
  // get
  void operator()(std::function<void(Trade const &)> const &) override;
  void operator()(std::function<void(Position const &)> const &) override;

  // put
  void operator()(std::span<Trade const> const &) override;

 private:
  std::unique_ptr<third_party::sqlite::Connection> connection_;
};

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
