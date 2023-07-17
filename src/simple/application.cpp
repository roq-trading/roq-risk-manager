/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/application.hpp"

#include <cassert>

#include "roq/client.hpp"

#include "simple/config.hpp"
#include "simple/settings.hpp"
#include "simple/shared.hpp"

using namespace std::literals;

namespace simple {

// === IMPLEMENTATION ===

int Application::main(roq::args::Parser const &args) {
  auto params = args.params();
  if (std::empty(params))
    roq::log::fatal("Unexpected"sv);
  Settings settings{args};
  settings.app.drop_copy = true;  // note!
  auto config = Config::parse_file(settings.config_file);
  roq::client::Trader{settings, config, params}.dispatch<value_type>(settings, config);
  return EXIT_SUCCESS;
}

}  // namespace simple
