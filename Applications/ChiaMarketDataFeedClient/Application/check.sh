#!/bin/bash
PREFIX="chia"
PID_FILE="pid.lock"

is_process_running() {
  kill -0 "$1" 2> /dev/null
  return $?
}

check_application() {
  local dir=$1
  local app_name="${dir#*_}"_"${dir%%_*}"
  cd "$dir" || return
  if [[ -f "$PID_FILE" ]]; then
    existing_pid=$(<"$PID_FILE")
    if is_process_running "$existing_pid"; then
      cd - > /dev/null
      return 0
    fi
  fi
  echo "$app_name is not running."
  cd - > /dev/null
  return 1
}

if [[ -n "$1" ]]; then
  target_dir="${PREFIX}_$1"
  if [[ -d "$target_dir" ]]; then
    check_application "$target_dir"
  else
    echo "Error: Directory $target_dir does not exist."
    exit 1
  fi
else
  for dir in "${PREFIX}"_*; do
    if [[ -d "$dir" ]]; then
      check_application "$dir"
    fi
  done
fi
