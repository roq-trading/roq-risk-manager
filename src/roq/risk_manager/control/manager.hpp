/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <functional>
#include <memory>

#include "roq/api.hpp"

#include "roq/io/context.hpp"

#include "roq/io/net/tcp/listener.hpp"

#include "roq/risk_manager/settings.hpp"

#include "roq/risk_manager/database/trade.hpp"

#include "roq/risk_manager/control/session.hpp"
#include "roq/risk_manager/control/shared.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Manager final : public Session::Handler, public io::net::tcp::Listener::Handler {
  struct Handler {
    virtual bool get_accounts() = 0;
    virtual bool get_trades_by_account(
        std::function<void(database::Trade const &)> const &,
        std::string_view const &account,
        std::chrono::nanoseconds start_time) = 0;
  };

  Manager(Handler &, Settings const &, io::Context &);

  Manager(Manager &&) = delete;
  Manager(Manager const &) = delete;

  void operator()(Event<Timer> const &);

 protected:
  // io::net::tcp::Listener::Handler
  void operator()(io::net::tcp::Connection::Factory &) override;

  // Session::Handler
  void operator()(Session::Disconnected const &) override;

  void remove_zombies();

 private:
  Handler &handler_;
  // io
  std::unique_ptr<io::net::tcp::Listener> listener_;
  // shared
  Shared shared_;
  // sessions
  uint64_t next_session_id_ = {};
  absl::flat_hash_map<uint64_t, std::unique_ptr<Session>> sessions_;
  std::chrono::nanoseconds next_cleanup_ = {};
  absl::flat_hash_set<uint64_t> zombies_;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
