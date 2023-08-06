/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/control/manager.hpp"

#include <charconv>
#include <filesystem>

#include "roq/logging.hpp"

#include "roq/io/network_address.hpp"

using namespace std::literals;
using namespace std::chrono_literals;

namespace roq {
namespace risk_manager {
namespace control {

// === CONSTANTS ===

namespace {
auto const CLEANUP_FREQUENCY = 1s;
}

// === HELPERS ===

namespace {
auto create_network_address(auto &settings) {
  auto address = settings.control_listen_address;
  uint16_t port = {};
  auto [_, ec] = std::from_chars(std::begin(address), std::end(address), port);
  if (ec == std::errc{}) {
    log::info(R"(The service will be started on port={})"sv, port);
    return io::NetworkAddress{port};
  }
  log::info(R"(The service will be started on path="{}")"sv, address);
  auto const directory = std::filesystem::path{address}.parent_path();
  if (!std::empty(directory) && std::filesystem::create_directory(directory))
    log::info(R"(Created path="{}")"sv, directory.c_str());
  return io::NetworkAddress{address};
}
}  // namespace

// === IMPLEMENTATION ===

Manager::Manager(Handler &handler, Settings const &settings, io::Context &context, database::Session &database)
    : handler_{handler}, listener_{context.create_tcp_listener(*this, create_network_address(settings))},
      shared_{settings}, database_{database} {
}

void Manager::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  if (next_cleanup_ < now) {
    next_cleanup_ = now + CLEANUP_FREQUENCY;
    remove_zombies();
  }
}

// io::net::tcp::Listener::Handler

void Manager::operator()(io::net::tcp::Connection::Factory &factory) {
  auto session_id = ++next_session_id_;
  auto session = std::make_unique<Session>(*this, session_id, factory, shared_, database_);
  sessions_.emplace(session_id, std::move(session));
}

// Session::Handler

void Manager::operator()(Session::Disconnected const &disconnected) {
  log::info("Detected zombie session"sv);
  zombies_.emplace(disconnected.session_id);
}

void Manager::remove_zombies() {
  auto count = std::size(zombies_);
  if (count == 0)
    return;
  for (auto iter : zombies_)
    sessions_.erase(iter);
  zombies_.clear();
  log::info("Removed {} zombied session(s) (remaining: {})"sv, count, std::size(sessions_));
}

}  // namespace control
}  // namespace risk_manager
}  // namespace roq
