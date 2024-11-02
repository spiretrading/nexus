#!/bin/bash
APPLICATION="SimulationMarketDataFeedClient"
LOG_DIR="./logs"

is_application_running() {
  ./check.sh > /dev/null
  return $?
}

mkdir -p "$LOG_DIR"
date_time=$(date '+%Y%m%d_%H_%M_%S')
log_name="srv_$date_time.log"
for existing_log in srv_*.log; do
  if [[ -f "$existing_log" ]]; then
    mv "$existing_log" "$LOG_DIR"
  fi
done
if is_application_running; then
  exit 0
fi
./$APPLICATION > "$log_name" 2>&1 &
new_pid=$!
echo "$new_pid" > "pid.lock"
