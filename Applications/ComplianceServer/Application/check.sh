#!/bin/bash
APPLICATION="ComplianceServer"
PID_FILE="pid.lock"

SHOW_RUNNING=false
FORCE_REPORT=false

usage() {
  cat <<EOF
Usage: $0 [-a] [-f] [-h]

Options:
  -a   Report only when $APPLICATION is running (silent otherwise)
  -f   Always report status (running or not)
  -h   Show this help
EOF
  exit 0
}

while getopts "afh" opt; do
  case "$opt" in
    a) SHOW_RUNNING=true ;;
    f) FORCE_REPORT=true ;;
    h) usage ;;
    *) usage ;;
  esac
done

is_process_running() {
  kill -0 "$1" 2>/dev/null
  return $?
}

if [[ -f "$PID_FILE" ]]; then
  existing_pid=$(<"$PID_FILE")
  if is_process_running "$existing_pid"; then
    if $SHOW_RUNNING || $FORCE_REPORT; then
      echo "$APPLICATION is running (pid $existing_pid)."
    fi
    exit 0
  fi
fi

# Not running
if $SHOW_RUNNING && ! $FORCE_REPORT; then
  exit 1
fi

echo "$APPLICATION is not running."
exit 1
