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
CONFIG_DIR="../../../share/"

DB_TYPE="sqlite"
DB_PARAMS="$CACHE_DIR/risk.sqlite3"

CONFIG_FILE="$CONFIG_DIR/config.toml"

mkdir -p "$CACHE_DIR"

$PREFIX "./roq-risk-manager" \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --db_type "$DB_TYPE" \
  --db_params "$DB_PARAMS" \
  --control_listen_address 1234 \
  $@
