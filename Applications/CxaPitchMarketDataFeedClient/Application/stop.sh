#!/bin/bash
PREFIX="cxa"
PID_FILE="pid.lock"

is_application_running() {
  pushd .. > /dev/null
  ./check.sh "$1" > /dev/null
  local result=$?
  popd > /dev/null
  return $result
}

wait_for_termination() {
  local pid=$1
  local timeout_tenths=3000
  local interval_tenths=1
  local max_interval_tenths=100
  local elapsed_tenths=0
  while((elapsed_tenths < timeout_tenths)); do
    if [[ ! -e /proc/$pid ]]; then
      return 0
    fi
    sleep "$((interval_tenths / 10)).$((interval_tenths % 10))"
    elapsed_tenths=$((elapsed_tenths + interval_tenths))
    interval_tenths=$((interval_tenths * 2))
    if((interval_tenths > max_interval_tenths)); then
      interval_tenths=$max_interval_tenths
    fi
  done
  return 1
}

stop_application() (
  local dir=$1
  local feed_name="${dir#*_}"
  local app_name="$feed_name"_"${dir%%_*}"
  cd "$dir" || return
  if [[ -f "$PID_FILE" ]]; then
    if is_application_running "$feed_name"; then
      local pid=$(<"$PID_FILE")
      if ! kill -SIGINT "$pid" 2> /dev/null; then
        echo "Error: Unable to signal $app_name (pid $pid)." >&2
        return 1
      fi
      if ! wait_for_termination "$pid"; then
        if ! kill -SIGKILL "$pid" 2> /dev/null ||
            ! wait_for_termination "$pid"; then
          echo "Error: Unable to terminate $app_name (pid $pid)." >&2
          return 1
        fi
        local log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)
        if [[ -n "$log_file" ]]; then
          echo "Forcefully terminated $app_name." >> "$log_file"
        fi
      fi
    fi
    rm -f "$PID_FILE"
  fi
)

if [[ -n "$1" ]]; then
  target_dir="${PREFIX}_$1"
  if [[ -d "$target_dir" ]]; then
    stop_application "$target_dir"
  else
    echo "Error: Directory $target_dir does not exist."
    exit 1
  fi
else
  status=0
  for dir in "${PREFIX}"_*; do
    if [[ -d "$dir" ]]; then
      stop_application "$dir" || status=1
    fi
  done
  exit "$status"
fi
