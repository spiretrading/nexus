#!/bin/bash
PREFIX="asxitch"
PID_FILE="pid.lock"
SHOW_RUNNING=false
FORCE_REPORT=false

is_process_running() {
  kill -0 "$1" 2> /dev/null
  return $?
}

usage() {
  cat <<EOF
Usage: $0 [-a] [-f] [-h] [service]

Options:
  -a   Report only when services are running (silent otherwise)
  -f   Always report status (running or not)
  -h   Show this help

Arguments:
  service   Check only the specified service (without prefix)
            If omitted, checks all ${PREFIX}_* services
EOF
  exit 0
}

check_application() {
  local dir=$1
  local app_name="${dir#*_}"_"${dir%%_*}"
  cd "$dir" || return
  if [[ -f "$PID_FILE" ]]; then
    existing_pid=$(<"$PID_FILE")
    if is_process_running "$existing_pid"; then
      if $SHOW_RUNNING || $FORCE_REPORT; then
        echo "$app_name is running (pid $existing_pid)."
      fi
      cd - > /dev/null
      return 0
    fi
  fi
  if $SHOW_RUNNING && ! $FORCE_REPORT; then
    cd - > /dev/null
    return 1
  fi
  echo "$app_name is not running."
  cd - > /dev/null
  return 1
}

while getopts "afh" opt; do
  case "$opt" in
    a) SHOW_RUNNING=true ;;
    f) FORCE_REPORT=true ;;
    h) usage ;;
    *) usage ;;
  esac
done
shift $((OPTIND - 1))

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
