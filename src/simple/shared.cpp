/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "simple/shared.hpp"

#include "roq/logging.hpp"

#include "simple/error.hpp"

using namespace std::literals;

namespace simple {

// === HELPERS ===

namespace {
template <typename R>
auto create_regex_symbols(auto &config) {
  using result_type = std::remove_cvref<R>::type;
  result_type result;
  for (auto &symbol : config.symbols) {
    third_party::re2::RegularExpression regular_expression{symbol};
    result.emplace_back(std::move(regular_expression));
  }
  return result;
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings, Config const &config)
    : settings{settings}, regex_symbols_{create_regex_symbols<decltype(regex_symbols_)>(config)} {
}

bool Shared::include(std::string_view const &symbol) const {
  for (auto &regex : regex_symbols_)
    if (regex.match(symbol))
      return true;
  return false;
}

std::string_view Shared::session_logon_helper(
    uint64_t session_id, std::string_view const &username, [[maybe_unused]] std::string_view const &password) {
  // XXX TODO validate password
  auto iter = username_to_session_.find(username);
  if (iter != std::end(username_to_session_))
    return Error::ALREADY_LOGGED_ON;
  roq::log::info(R"(Adding session_id={}, username="{}")"sv, session_id, username);
  auto res_1 = username_to_session_.try_emplace(username, session_id);
  assert(res_1.second);
  auto &username_1 = (*res_1.first).first;
  auto res_2 = session_to_username_.try_emplace(session_id, username_1);
  assert(res_2.second);
  return {};
}

std::string_view Shared::session_logout_helper(uint64_t session_id) {
  auto iter = session_to_username_.find(session_id);
  if (iter == std::end(session_to_username_))
    return Error::NOT_LOGGED_ON;
  auto &username = (*iter).second;
  roq::log::info(R"(Removing session_id={}, username="{}")"sv, session_id, username);
  username_to_session_.erase(username);
  session_to_username_.erase(iter);
  return {};
}

void Shared::session_remove_helper(uint64_t session_id) {
  auto iter = session_to_username_.find(session_id);
  if (iter == std::end(session_to_username_)) {
    roq::log::info("Removing session_id={}..."sv, session_id);
    return;
  }
  auto &username = (*iter).second;
  roq::log::info(R"(Removing session_id={}, username="{}")"sv, session_id, username);
  username_to_session_.erase((*iter).second);
  session_to_username_.erase(iter);
}

}  // namespace simple
