#!/bin/bash
APPLICATION="MarketDataServer"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"

wait_for_server() {
  local server_name="$1"
  local addresses=($(yq -r \
    ".$server_name.interface? | select(. != null),
     .$server_name.addresses[]? | select(. != null) | @sh" "$CONFIG_FILE"))
  if [[ ${#addresses[@]} -eq 0 ]]; then
    return 0
  fi
  while true; do
    if ! ./check.sh > /dev/null; then
      exit 1
    fi
    for addr in "${addresses[@]}"; do
      host="${addr%%:*}"
      port="${addr##*:}"
      if ss -ltn | grep -qE "$host:$port"; then
        return 0
      fi
    done
    sleep 0.5
  done
}

if ./check.sh > /dev/null; then
  exit 0
fi
mkdir -p "$LOG_DIR"
date_time=$(date '+%Y%m%d_%H_%M_%S')
log_name="srv_$date_time.log"
for existing_log in srv_*.log; do
  if [[ -f "$existing_log" ]]; then
    mv "$existing_log" "$LOG_DIR"
  fi
done
./$APPLICATION > "$log_name" 2>&1 &
new_pid=$!
echo "$new_pid" > "pid.lock"
wait_for_server registry_server
wait_for_server feed_server
