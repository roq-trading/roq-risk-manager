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
#include "roq/risk_manager/shared.hpp"

#include "roq/risk_manager/database/session.hpp"

#include "roq/risk_manager/control/session.hpp"
#include "roq/risk_manager/control/shared.hpp"

namespace roq {
namespace risk_manager {
namespace control {

struct Manager final : public Session::Handler, public io::net::tcp::Listener::Handler {
  struct Handler {};

  Manager(Handler &, Settings const &, io::Context &, risk_manager::Shared const &, database::Session &);

  Manager(Manager &&) = default;
  Manager(Manager const &) = delete;

  void operator()(Event<Timer> const &);

  void operator()(database::Trade const &);

 protected:
  // io::net::tcp::Listener::Handler
  void operator()(io::net::tcp::Connection::Factory &) override;

  // Session::Handler
  void operator()(Session::Disconnected const &) override;
  void operator()(Session::Upgraded const &) override;

  void remove_zombies();

 private:
  Handler &handler_;
  // io
  std::unique_ptr<io::net::tcp::Listener> listener_;
  // shared
  Shared shared_;
  risk_manager::Shared const &shared_2_;
  database::Session &database_;
  // sessions
  uint64_t next_session_id_ = {};
  absl::flat_hash_map<uint64_t, std::unique_ptr<Session>> sessions_;
  absl::flat_hash_set<uint64_t> subscribers_;
  std::chrono::nanoseconds next_cleanup_ = {};
  absl::flat_hash_set<uint64_t> zombies_;
};

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
