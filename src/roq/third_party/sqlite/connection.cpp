/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/third_party/sqlite/connection.hpp"

#include <cassert>
#include <string>

#include "roq/exceptions.hpp"

#include "roq/logging.hpp"

#include "roq/third_party/sqlite/statement.hpp"

using namespace std::literals;

using namespace fmt::literals;

namespace roq {
namespace third_party {
namespace sqlite {

// === HELPERS ===
namespace {
using value_type = Connection::value_type;

void deleter(value_type *ptr) {
  if (ptr)
    sqlite3_close(ptr);
}

template <typename R>
R create(std::string const &filename) {
  value_type *handle = nullptr;
  auto result = sqlite3_open(filename.c_str(), &handle);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_open: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
  return R{handle, deleter};
}
}  // namespace

// === IMPLEMENTATION ===
//
Connection::Connection(std::string_view const &filename) : handle_(create<decltype(handle_)>(std::string{filename})) {
}

void Connection::exec(std::string_view const &query) {
  Statement statement{*this, query};
  if (statement.step())
    throw RuntimeError{"Statement is not complete"sv};
}

bool Connection::table_exists(std::string_view const &name) {
  auto query = fmt::format("SELECT name FROM sqlite_master WHERE type='table' AND name='{}'"_cf, name);
  log::debug(R"(query="{}")"sv, query);
  Statement statement{*this, query};
  // note! we don't need to check the result: any result means the table exists
  return statement.step();
}

void Connection::rename_table(std::string_view const &old_name, std::string_view const &new_name) {
  auto query = fmt::format("ALTER TABLE {} RENAME TO {}"_cf, old_name, new_name);
  log::debug(R"(query="{}")"sv, query);
  Statement statement{*this, query};
  auto result = statement.step();
  log::debug("{}"sv, result);
}

void Connection::drop_table(std::string_view const &name) {
  auto query = fmt::format("DROP TABLE {}"_cf, name);
  log::debug(R"(query="{}")"sv, query);
  Statement statement{*this, query};
  auto result = statement.step();
  log::debug("{}"sv, result);
}

}  // namespace sqlite
}  // namespace third_party
}  // namespace roq
