/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/third_party/sqlite/statement.hpp"

#include <cassert>
#include <string>

#include "roq/api.hpp"
#include "roq/exceptions.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace third_party {
namespace sqlite {

// === HELPERS ===

namespace {
using value_type = Statement::value_type;

void deleter(value_type *ptr) {
  if (ptr)
    sqlite3_finalize(ptr);
}

template <typename R>
R create(auto &connection, auto &query) {
  value_type *handle = nullptr;
  auto result = sqlite3_prepare(connection, std::data(query), std::size(query), &handle, nullptr);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_prepare: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
  return R{handle, deleter};
}
}  // namespace

// === IMPLEMENTATION ===

Statement::Statement(Connection &connection, std::string_view const &query)
    : handle_(create<decltype(handle_)>(connection, query)) {
}

// reset

void Statement::reset() {
  auto result = sqlite3_reset(*this);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_reset: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

// step

bool Statement::step() {
  auto result = sqlite3_step(*this);
  switch (result) {
    case SQLITE_DONE:
      return false;
    case SQLITE_ROW:
      return true;
    default:
      throw RuntimeError{R"(sqlite3_bind_null: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
  }
}

// bind

void Statement::bind(size_t column) {
  auto real_column = static_cast<int>(column) + 1;
  auto result = sqlite3_bind_null(*this, real_column);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_bind_null: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

void Statement::bind(size_t column, int32_t value) {
  auto real_column = static_cast<int>(column) + 1;
  auto result = sqlite3_bind_int(*this, real_column, value);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_bind_int: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

void Statement::bind(size_t column, int64_t value) {
  auto real_column = static_cast<int>(column) + 1;
  auto result = sqlite3_bind_int64(*this, real_column, value);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_bind_int64: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

void Statement::bind(size_t column, double value) {
  auto real_column = static_cast<int>(column) + 1;
  auto result = sqlite3_bind_double(*this, real_column, value);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_bind_double: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

void Statement::bind(size_t column, std::string_view const &value) {
  auto real_column = static_cast<int>(column) + 1;
  auto result = sqlite3_bind_text(*this, real_column, std::data(value), std::size(value), nullptr);
  if (result != SQLITE_OK)
    throw RuntimeError{R"(sqlite3_bind_text: result={} ("{}"))"sv, result, sqlite3_errstr(result)};
}

bool Statement::is_null(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  return type == SQLITE_NULL;
}

template <>
int32_t Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return {};
    case SQLITE_INTEGER:
      return sqlite3_column_int(*this, static_cast<int>(column));
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

template <>
uint32_t Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return {};
    case SQLITE_INTEGER: {
      auto result = sqlite3_column_int64(*this, static_cast<int>(column));
      if (result < 0 || result > std::numeric_limits<uint32_t>::max())
        throw RuntimeError{"Unexpected: value={} exceeds maximum of uint32"sv, result};
      return static_cast<uint32_t>(result);
    }
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

template <>
int64_t Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return {};
    case SQLITE_INTEGER:
      return sqlite3_column_int64(*this, static_cast<int>(column));
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

template <>
uint64_t Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return {};
    case SQLITE_INTEGER: {
      auto tmp = sqlite3_column_int64(*this, static_cast<int>(column));
      if (tmp < 0)
        throw RuntimeError{R"(Unexpected: value={} is negative)"sv, tmp};
      return static_cast<uint64_t>(tmp);
    }
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

template <>
double Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return NaN;
    case SQLITE_INTEGER: {
      auto tmp = sqlite3_column_int64(*this, static_cast<int>(column));
      return static_cast<double>(tmp);
    }
    case SQLITE_FLOAT:
      return sqlite3_column_double(*this, static_cast<int>(column));
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

template <>
std::string Statement::get(size_t column) {
  auto type = sqlite3_column_type(*this, static_cast<int>(column));
  switch (type) {
    case SQLITE_NULL:
      return {};
    case SQLITE_TEXT: {
      auto result = sqlite3_column_text(*this, static_cast<int>(column));
      auto length = sqlite3_column_bytes(*this, static_cast<int>(column));
      if (length < 0)
        throw RuntimeError{R"(Unexpected: length={})"sv, length};
      return std::string{reinterpret_cast<char const *>(result), static_cast<size_t>(length)};
    }
    default:
      throw RuntimeError{R"(Unexpected: type={})"sv, type};
  }
}

}  // namespace sqlite
}  // namespace third_party
}  // namespace roq
