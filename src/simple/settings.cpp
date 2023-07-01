/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/settings.hpp"

#include "simple/flags/flags.hpp"

using namespace std::chrono_literals;

namespace simple {

// === CONSTANTS ===

namespace {
auto CONNECTION_TIMEOUT = 5s;
auto TLS_VALIDATE_CERTIFICATE = false;
}  // namespace

// === IMPLEMENTATION ===

Settings Settings::create() {
  return {
      .config_file = flags::Flags::config_file(),
      .net{
          .connection_timeout = CONNECTION_TIMEOUT,
          .tls_validate_certificate = TLS_VALIDATE_CERTIFICATE,
      },
      .fix{
          .username = flags::Flags::fix_username(),
          .password = flags::Flags::fix_password(),
          .sender_comp_id = flags::Flags::fix_sender_comp_id(),
          .target_comp_id = flags::Flags::fix_target_comp_id(),
          .decode_buffer_size = flags::Flags::fix_decode_buffer_size(),
          .encode_buffer_size = flags::Flags::fix_encode_buffer_size(),
          .ping_freq = flags::Flags::fix_ping_freq(),
          .debug = flags::Flags::fix_debug(),
          .market_depth = flags::Flags::fix_market_depth(),
      },
      .json{
          .listen_port = flags::Flags::json_listen_port(),
          .url_prefix = flags::Flags::json_url_prefix(),
      },
      .test{
          .disable_market_data = flags::Flags::disable_market_data(),
      },
  };
}

}  // namespace simple
