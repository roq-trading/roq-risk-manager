/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <sqlite3.h>

#include <memory>
#include <string_view>

namespace roq {
namespace third_party {
namespace sqlite {

struct Connection final {
  using value_type = struct sqlite3;

  explicit Connection(std::string_view const &filename);

  operator value_type *() { return handle_.get(); }
  operator value_type const *() const { return handle_.get(); }

  void exec(std::string_view const &query);

  // utilities

  bool table_exists(std::string_view const &name);
  void rename_table(std::string_view const &old_name, std::string_view const &new_name);
  void drop_table(std::string_view const &name);

 private:
  std::unique_ptr<value_type, void (*)(value_type *)> handle_;
};

}  // namespace sqlite
}  // namespace third_party
}  // namespace roq
