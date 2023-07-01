#!/usr/bin/env bash
./fix-bridge-client-simple \
  --config_file config/test.toml \
  --fix_target_comp_id "roq-fix-bridge" \
  --fix_sender_comp_id "roq-fix-client-test" \
  --fix_username "trader" \
  --json_listen_port 2345 \
  tcp://localhost:3456 \
  $@
