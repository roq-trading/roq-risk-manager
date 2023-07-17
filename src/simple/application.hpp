/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/service.hpp"

#include "simple/controller.hpp"

namespace simple {

struct Application final : public roq::Service {
  using Service::Service;  // inherit constructors

 protected:
  int main(roq::args::Parser const &) override;

 private:
  using value_type = Controller;  // note!
};

}  // namespace simple
