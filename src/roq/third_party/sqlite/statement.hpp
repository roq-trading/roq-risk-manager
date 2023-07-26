/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <sqlite3.h>

#include <memory>
#include <string_view>

#include "roq/third_party/sqlite/connection.hpp"

namespace roq {
namespace third_party {
namespace sqlite {

struct Statement final {
  using value_type = struct sqlite3_stmt;

  Statement(Connection &, std::string_view const &query);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  void reset();

  // returns false when done
  bool step();

  // note! columns always use zero-based index (sqlite3 expects one-based for bind and zero-based otherwise...!)

  void bind(size_t column);  // null
  void bind(size_t column, int32_t value);
  void bind(size_t column, int64_t value);
  void bind(size_t column, double value);
  void bind(size_t column, std::string_view const &value);

  bool is_null(size_t column);

  template <typename T>
  T get(size_t column);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace sqlite
}  // namespace third_party
}  // namespace roq
