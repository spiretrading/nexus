#!/bin/bash
TARGET="CxaPitchMarketDataFeedClient"
PREFIX="cxa"
LOG_DIR="./logs"

is_application_running() {
  pushd .. > /dev/null
  ./check.sh "$1" > /dev/null
  result=$?
  popd > /dev/null
  return $result
}

start_application() (
  local dir=$1
  local feed_name="${dir#*_}"
  local app_name="$feed_name"_"${dir%%_*}"
  local log_name="srv_$(date '+%Y%m%d_%H_%M_%S').log"
  cd "$dir" || return
  if is_application_running "$feed_name"; then
    return 0
  fi
  if [[ ! -e "$app_name" && ! -L "$app_name" ]]; then
    ln -s ../"$TARGET" "$app_name" || return 1
  fi
  if [[ ! -f "$app_name" || ! -x "$app_name" ]]; then
    echo "Error: $dir/$app_name is missing or not executable." >&2
    return 1
  fi
  if [[ ! -f config.yml ]]; then
    echo "Error: $dir/config.yml does not exist." >&2
    return 1
  fi
  mkdir -p "$LOG_DIR" || return 1
  for existing_log in srv_*.log; do
    if [[ -f "$existing_log" ]]; then
      mv "$existing_log" "$LOG_DIR" || return 1
    fi
  done
  : > "$log_name" || return 1
  local pid
  {
    ./"$app_name" > "$log_name" 2>&1 &
    pid=$!
    echo "$pid"
  } > pid.lock || return 1
  sleep 1
  if ! kill -0 "$pid" 2> /dev/null; then
    wait "$pid"
    rm -f pid.lock
    echo "Error: $app_name exited during startup; see $dir/$log_name." >&2
    return 1
  fi
)

if [[ -n "$1" ]]; then
  target_dir="${PREFIX}_$1"
  if [[ -d "$target_dir" ]]; then
    start_application "$target_dir"
  else
    echo "Error: Directory $target_dir does not exist."
    exit 1
  fi
else
  status=0
  for dir in "${PREFIX}"_*; do
    if [[ -d "$dir" ]]; then
      start_application "$dir" || status=1
    fi
  done
  exit "$status"
fi
