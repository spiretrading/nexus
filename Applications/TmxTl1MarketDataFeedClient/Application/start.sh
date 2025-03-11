#!/bin/bash
TARGET="TmxTl1MarketDataFeedClient"
PREFIX="tmxtl1"
LOG_DIR="./logs"

is_application_running() {
  pushd .. > /dev/null
  ./check.sh "$1" > /dev/null
  result=$?
  popd > /dev/null
  return $result
}

start_application() {
  local dir=$1
  local feed_name="${dir#*_}"
  local app_name="$feed_name"_"${dir%%_*}"
  local log_name="srv_$(date '+%Y%m%d_%H_%M_%S').log"
  cd "$dir" || return
  if is_application_running "$feed_name"; then
    cd - > /dev/null
    return 0
  fi
  mkdir -p "$LOG_DIR"
  for existing_log in srv_*.log; do
    if [[ -f "$existing_log" ]]; then
      mv "$existing_log" "$LOG_DIR"
    fi
  done
  if [[ ! -f "$app_name" ]]; then
    ln -s ../"$TARGET" "$app_name"
  fi
  ./"$app_name" > "$log_name" 2>&1 &
  new_pid=$!
  echo "$new_pid" > "pid.lock"
  cd - > /dev/null
}

if [[ -n "$1" ]]; then
  target_dir="${PREFIX}_$1"
  if [[ -d "$target_dir" ]]; then
    start_application "$target_dir"
  else
    echo "Error: Directory $target_dir does not exist."
    exit 1
  fi
else
  for dir in "${PREFIX}"_*; do
    if [[ -d "$dir" ]]; then
      start_application "$dir"
    fi
  done
fi
