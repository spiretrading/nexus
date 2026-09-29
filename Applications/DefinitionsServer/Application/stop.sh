#!/bin/bash
APPLICATION="DefinitionsServer"
PID_FILE="pid.lock"
trap 'exit 1' HUP INT TERM

wait_for_termination() {
  local deadline=$((SECONDS + 300))
  local interval_tenths=1
  local max_interval_tenths=100
  local status
  while((SECONDS < deadline)); do
    status=0
    ./check.sh -p "$pid" > /dev/null || status=$?
    if((status == 1)); then
      return 0
    elif((status != 0)); then
      return 2
    fi
    sleep "$((interval_tenths / 10)).$((interval_tenths % 10))"
    interval_tenths=$((interval_tenths * 2))
    if((interval_tenths > max_interval_tenths)); then
      interval_tenths=$max_interval_tenths
    fi
  done
  return 1
}

if [[ ! -f "$PID_FILE" ]]; then
  exit 0
fi
pid=$(<"$PID_FILE")
status=0
./check.sh -p "$pid" > /dev/null || status=$?
if((status == 1)); then
  exit 0
elif((status != 0)); then
  exit "$status"
fi
if ! kill -SIGTERM "$pid" 2> /dev/null; then
  echo "Error: Unable to signal $APPLICATION (pid $pid)." >&2
  exit 1
fi
status=0
wait_for_termination || status=$?
if((status == 2)); then
  exit 1
elif((status != 0)); then
  status=0
  ./check.sh -p "$pid" > /dev/null || status=$?
  if((status == 0)); then
    if ! kill -SIGKILL "$pid" 2> /dev/null || ! wait_for_termination; then
      echo "Error: Unable to terminate $APPLICATION (pid $pid)." >&2
      exit 1
    fi
    log_file=$(ls -t srv_*.log 2> /dev/null | head -n 1)
    if [[ -n "$log_file" ]]; then
      echo "Forcefully terminated $APPLICATION." >> "$log_file"
    fi
  elif((status != 1)); then
    exit "$status"
  fi
fi
exit 0
