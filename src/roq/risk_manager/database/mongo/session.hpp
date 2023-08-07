/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <memory>

#include <mongocxx/instance.hpp>

#include "roq/risk_manager/database/session.hpp"

namespace roq {
namespace risk_manager {
namespace database {
namespace mongo {

struct Session final : public database::Session {
  explicit Session(std::string_view const &params);

 protected:
  // query
  void operator()(std::function<void(Account const &)> const &) override;
  void operator()(std::function<void(Position const &)> const &) override;
  void operator()(
      std::function<void(Trade const &)> const &,
      std::string_view const &account,
      std::chrono::nanoseconds start_time) override;

  // insert
  void operator()(std::span<Trade const> const &) override;

 private:
  mongocxx::instance instance_;
};

}  // namespace mongo
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
