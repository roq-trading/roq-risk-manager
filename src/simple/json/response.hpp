/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/rest/server.hpp"

namespace simple {
namespace json {

// helper

struct Response final {
  Response(roq::web::rest::Server &, roq::web::rest::Server::Request const &, std::string &encode_buffer);

  template <typename... Args>
  inline void operator()(
      roq::web::http::Status status,
      roq::web::http::ContentType content_type,
      fmt::format_string<Args...> const &fmt,
      Args &&...args) {
    encode_buffer_.clear();
    fmt::format_to(std::back_inserter(encode_buffer_), fmt, std::forward<Args>(args)...);
    send(status, content_type, encode_buffer_);
  }

 protected:
  void send(roq::web::http::Status, roq::web::http::ContentType, std::string_view const &body);

 private:
  roq::web::rest::Server &server_;
  roq::web::rest::Server::Request const &request_;
  std::string &encode_buffer_;
};

}  // namespace json
}  // namespace simple
