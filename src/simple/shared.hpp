/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <string>
#include <vector>

#include "simple/config.hpp"

#include "third_party/re2/regular_expression.hpp"

#include "simple/settings.hpp"

namespace simple {

struct Shared final {
  Shared(Settings const &, Config const &);

  absl::flat_hash_set<std::string> symbols;

  bool include(std::string_view const &symbol) const;

  Settings const &settings;

  std::string encode_buffer;

  template <typename Success, typename Failure>
  void session_logon(
      uint64_t session_id,
      std::string_view const &username,
      std::string_view const &password,
      Success success,
      Failure failure) {
    auto result = session_logon_helper(session_id, username, password);
    if (std::empty(result))
      success();
    else
      failure(result);
  }

  template <typename Success, typename Failure>
  void session_logout(uint64_t session_id, Success success, Failure failure) {
    auto result = session_logout_helper(session_id);
    if (std::empty(result))
      success();
    else
      failure(result);
  }

  void session_remove(uint64_t session_id) { sessions_to_remove_.emplace(session_id); }

  template <typename Callback>
  void session_cleanup(Callback callback) {
    for (auto session_id : sessions_to_remove_) {
      session_remove_helper(session_id);
      callback(session_id);
    }
    sessions_to_remove_.clear();
  }

  template <typename Callback>
  bool session_find(std::string_view const &username, Callback callback) {
    auto iter = username_to_session_.find(username);
    if (iter == std::end(username_to_session_))
      return false;
    callback((*iter).second);
    return true;
  }

 protected:
  std::string_view session_logon_helper(
      uint64_t session_id, std::string_view const &username, std::string_view const &password);
  std::string_view session_logout_helper(uint64_t session_id);
  void session_remove_helper(uint64_t session_id);

  absl::flat_hash_map<std::string, uint64_t> username_to_session_;
  absl::flat_hash_map<uint64_t, std::string> session_to_username_;
  absl::flat_hash_set<uint64_t> sessions_to_remove_;

 private:
  std::vector<third_party::re2::RegularExpression> const regex_symbols_;
};

}  // namespace simple
