/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace simple {
namespace flags {

struct Flags final {
  static std::string_view config_file();
  // fix
  static std::string_view fix_target_comp_id();
  static std::string_view fix_sender_comp_id();
  static std::string_view fix_username();
  static std::string_view fix_password();
  static uint32_t fix_decode_buffer_size();
  static uint32_t fix_encode_buffer_size();
  static std::chrono::seconds fix_ping_freq();
  static bool fix_debug();
  static uint32_t fix_market_depth();
  // json
  static uint16_t json_listen_port();
  static std::string_view json_url_prefix();
  // test
  static bool disable_market_data();
};

}  // namespace flags
}  // namespace simple
