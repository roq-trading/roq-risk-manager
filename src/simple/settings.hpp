/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string_view>

namespace simple {

struct Settings final {
  static Settings create();

  std::string_view config_file;

  struct {
    std::chrono::nanoseconds connection_timeout = {};
    bool tls_validate_certificate = {};
  } net;

  struct {
    std::string_view username;
    std::string_view password;
    std::string_view sender_comp_id;
    std::string_view target_comp_id;
    uint32_t decode_buffer_size = {};
    uint32_t encode_buffer_size = {};
    std::chrono::seconds ping_freq = {};
    bool debug = {};
    uint32_t market_depth = {};
  } fix;

  struct {
    uint16_t listen_port = {};
    std::string_view url_prefix;
  } json;

  struct {
    bool disable_market_data = {};
  } test;
};

}  // namespace simple
