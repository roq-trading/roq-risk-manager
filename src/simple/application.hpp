/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/service.hpp"

namespace simple {

struct Application final : public roq::Service {
  using Service::Service;  // inherit constructors

 protected:
  int main(int argc, char **argv) override;

  int main_helper(std::span<std::string_view> const &args);
};

}  // namespace simple
