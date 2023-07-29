#!/usr/bin/env bash

NAME="risk"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-risk-manager/$CONFIG.toml"

echo "CONFIG_FILE=$CONFIG_FILE"

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

DB_TYPE="sqlite3"
DB_PARAMS="$CACHE_DIR/risk.sqlite3"

mkdir -p "$CACHE_DIR"

$PREFIX "./roq-risk-manager" \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --db_type "$DB_TYPE" \
  --db_params "$DB_PARAMS" \
  --control_listen_address 1234 \
  $@
