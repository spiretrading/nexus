#!/bin/bash
PREFIX="tmxtl1"
PID_FILE="pid.lock"

is_application_running() {
  pushd .. > /dev/null
  ./check.sh "$1" > /dev/null
  result=$?
  popd > /dev/null
  return $result
}

wait_for_termination() {
  local pid=$1
  local timeout=300
  local interval=0.1
  local max_interval=10
  local elapsed=0
  while(( $(echo "$elapsed < $timeout" | bc -l) )); do
    if [[ ! -e /proc/$pid ]]; then
      return 0
    fi
    sleep $interval
    elapsed=$(echo "$elapsed + $interval" | bc)
    interval=$(echo "$interval * 2" | bc)
    if(( $(echo "$interval > $max_interval" | bc -l) )); then
      interval=$max_interval
    fi
  done
  return 1
}

stop_application() {
  local dir=$1
  local feed_name="${dir#*_}"
  local app_name="$feed_name"_"${dir%%_*}"
  cd "$dir" || return
  if [[ -f "$PID_FILE" ]]; then
    if is_application_running "$feed_name"; then
      local pid=$(<"$PID_FILE")
      kill -SIGINT "$pid" 2> /dev/null
      if ! wait_for_termination "$pid"; then
        kill -SIGKILL "$pid" > /dev/null
        log_file=$(ls -t srv_*.log 2>/dev/null | head -n 1)
        if [[ -n "$log_file" ]]; then
         echo "Forcefully terminated $APPLICATION." >> "$log_file"
        fi
      fi
    fi
    rm -f "$PID_FILE"
  fi
  cd - > /dev/null
}

if [[ -n "$1" ]]; then
  target_dir="${PREFIX}_$1"
  if [[ -d "$target_dir" ]]; then
    stop_application "$target_dir"
  else
    echo "Error: Directory $target_dir does not exist."
    exit 1
  fi
else
  for dir in "${PREFIX}"_*; do
    if [[ -d "$dir" ]]; then
      stop_application "$dir"
    fi
  done
fi
