#!/bin/bash
APPLICATION="ComplianceServer"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"
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
all_addresses=($(yq -r \
  '.server.interface? | select(. != null), .server.addresses[]? | select(. != null) | @sh' "$CONFIG_FILE"))
if [[ ${#all_addresses[@]} -eq 0 ]]; then
  exit 0
fi

is_any_address_listening() {
  for addr in "${all_addresses[@]}"; do
    host="${addr%%:*}"
    port="${addr##*:}"
    if ss -ltn | grep -qE "$host:$port"; then
      return 0
    fi
  done
  return 1
}

while true; do
  if ! ./check.sh > /dev/null; then
    exit 1
  fi
  if is_any_address_listening; then
    break
  fi
  sleep 0.5
done
