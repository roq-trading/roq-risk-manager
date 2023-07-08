/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/application.hpp"

#include <cassert>
#include <vector>

#include "roq/client.hpp"

#include "simple/config.hpp"
#include "simple/settings.hpp"
#include "simple/shared.hpp"

using namespace std::chrono_literals;
using namespace std::literals;

namespace simple {

// === IMPLEMENTATION ===

int Application::main(int argc, char **argv) {
  std::vector<std::string_view> args;
  for (int i = 0; i < argc; ++i)
    args.emplace_back(argv[i]);
  return main_helper(args);
}

int Application::main_helper(std::span<std::string_view> const &args) {
  assert(!std::empty(args));
  if (std::size(args) == 1)
    roq::log::fatal("Unexpected"sv);
  Settings settings;
  Config config{settings};
  Shared shared;
  auto connections = args.subspan(1);
  roq::client::Trader{config, connections}.dispatch<value_type>(shared);
  return EXIT_SUCCESS;
}

}  // namespace simple
