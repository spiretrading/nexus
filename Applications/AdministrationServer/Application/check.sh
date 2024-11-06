#!/bin/bash
APPLICATION="AdministrationServer"
PID_FILE="pid.lock"

is_process_running() {
  kill -0 "$1" 2> /dev/null
  return $?
}

if [[ -f "$PID_FILE" ]]; then
  existing_pid=$(<"$PID_FILE")
  if is_process_running "$existing_pid"; then
    exit 0
  fi
fi
echo "$APPLICATION is not running."
exit 1
