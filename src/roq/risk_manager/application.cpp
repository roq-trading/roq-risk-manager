/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/risk_manager/application.hpp"

#include <cassert>

#include "roq/client.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/risk_manager/config.hpp"
#include "roq/risk_manager/settings.hpp"
#include "roq/risk_manager/shared.hpp"

using namespace std::literals;

namespace roq {
namespace risk_manager {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  auto params = args.params();
  if (std::empty(params))
    log::fatal("Unexpected"sv);
  Settings settings{args};
  settings.app.drop_copy = true;  // note!
  auto config = Config::parse_file(settings.config_file);
  auto context = roq::io::engine::ContextFactory::create_libevent();
  client::Bridge{settings, config, params}.dispatch<value_type>(settings, config, *context, std::size(params));
  return EXIT_SUCCESS;
}

}  // namespace risk_manager
}  // namespace roq
