#!/usr/bin/env bash

set -x

echo -e "\033[1;34m--- ENV ---\033[0m"

env | sort

echo -e "\033[1;34m--- CHECK ---\033[0m"

if [[ "$build_platform" != "$target_platform" ]]; then
  echo -e "Drop testing when cross-compiling..." && exit 0
fi

echo -e "\033[1;34m--- BINARY ---\033[0m"

if "$PKG_NAME" --help; then  # note! abseil flags returns 1
  (>&2 echo -e "\033[1;31mERROR: Unexpected error code.\033[0m") && exit 1
fi

echo -e "\033[1;34m--- BENCHMARK ---\033[0m"

if which "$PKG_NAME-benchmark"; then
  if ! "$PKG_NAME-benchmark" --help; then  # note! benchmark returns 0
    (>&2 echo -e "\033[1;31mERROR: Unexpected error code.\033[0m") && exit 1
  fi
fi

echo -e "\033[1;34m--- DONE ---\033[0m"
