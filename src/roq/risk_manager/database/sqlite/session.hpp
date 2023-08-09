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
  // query
  void operator()(std::function<void(Account const &)> const &) override;
  void operator()(std::function<void(Position const &)> const &) override;
  void operator()(
      std::function<void(Trade const &)> const &,
      std::string_view const &account,
      std::chrono::nanoseconds start_time) override;
  void operator()(
      std::function<void(Funds const &)> const &,
      std::string_view const &account,
      std::string_view const &currency) override;

  // insert
  void operator()(std::span<Trade const> const &) override;
  void operator()(std::span<Correction const> const &) override;
  void operator()(std::span<Funds const> const &) override;

  // maintenance
  void operator()(Compress const &) override;

 private:
  std::unique_ptr<third_party::sqlite::Connection> connection_;
};

}  // namespace sqlite
}  // namespace database
}  // namespace risk_manager
}  // namespace roq
