#!/usr/bin/env bash

NAME="risk"

# debug?

if [ "$1" == "debug" ]; then
  KERNEL="$(uname -a)"
  case "$KERNEL" in
    Linux*)
      PREFIX="gdb --args"
      ;;
    Darwin*)
      PREFIX="lldb --"
      ;;
  esac
  shift 1
else
  PREFIX=
fi

CACHE_DIR="$HOME/var/lib/roq/cache/risk-manager"

mkdir -p "$CACHE_DIR"

$PREFIX "./roq-risk-manager" \
  --name "$NAME" \
  --config_file "config/$NAME.toml" \
  --db_type "sqlite" \
  --db_params "$CACHE_DIR/risk.sqlite3" \
  --control_listen_address 1234 \
  $@
