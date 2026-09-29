#!/bin/bash
APPLICATION="SimulationMarketDataFeedClient"
CONFIG_FILE="config.yml"
LOG_DIR="./logs"
LOCK_CONFLICT=75
platform=$(uname -s) || exit 1
if [[ "$platform" == "Linux" ]]; then
  lock_command=(flock -n -E "$LOCK_CONFLICT")
else
  lock_command=(lockf -s -t 0)
fi
has_lock_conflict=false
if command -v "${lock_command[0]}" > /dev/null &&
    { exec 9>> .instance.lock; } 2> /dev/null; then
  lock_status=0
  "${lock_command[@]}" 9 2> /dev/null || lock_status=$?
  if((lock_status != 0)); then
    exec 9>&-
    if((lock_status == LOCK_CONFLICT)); then
      has_lock_conflict=true
    fi
  fi
fi
trap 'exit 1' HUP INT TERM
pid=""
status=0
./check.sh > /dev/null || status=$?
if((status == 0)); then
  pid=$(<pid.lock)
elif((status != 1)); then
  exit "$status"
elif $has_lock_conflict; then
  echo "Error: $APPLICATION is already starting or running." >&2
  exit 1
fi
if [[ ! -f "$APPLICATION" || ! -x "$APPLICATION" ]]; then
  echo "Error: $APPLICATION is missing or not executable." >&2
  exit 1
fi
if [[ ! -f "$CONFIG_FILE" ]]; then
  echo "Error: $CONFIG_FILE does not exist." >&2
  exit 1
fi
if [[ "$platform" != "Linux" ]] && ! command -v lsof > /dev/null; then
  echo "Error: lsof is required to identify $APPLICATION." >&2
  exit 1
fi
log_name="srv_*.log"
if [[ -z "$pid" ]]; then
  mkdir -p "$LOG_DIR" || exit 1
  for existing_log in srv_*.log; do
    if [[ -f "$existing_log" ]]; then
      if [[ ! -s "$existing_log" ]]; then
        rm "$existing_log" || exit 1
        continue
      fi
      archive="$LOG_DIR/${existing_log##*/}"
      if [[ -e "$archive" ]]; then
        archive=$(mktemp "$archive.XXXXXX") || exit 1
      fi
      mv "$existing_log" "$archive" || exit 1
    fi
  done
  log_name="srv_$(date '+%Y%m%d_%H_%M_%S')_$$.log"
  (set -o noclobber; : > "$log_name") || exit 1
  bash -c '
    echo "$$" > pid.lock || exit 1
    exec "$@"
  ' bash "./$APPLICATION" > "$log_name" 2>&1 &
  pid=$!
fi

deadline=$((SECONDS + 30))
while((SECONDS < deadline)); do
  status=0
  ./check.sh -p "$pid" > /dev/null || status=$?
  if((status == 1)); then
    if ! kill -0 "$pid" 2> /dev/null; then
      wait "$pid" 2> /dev/null
      echo "Error: $APPLICATION exited during startup; see $log_name." >&2
      exit 1
    fi
    sleep 0.5
    continue
  elif((status != 0)); then
    exit "$status"
  fi
  exit 0
done
echo "Error: $APPLICATION startup timed out; see $log_name." >&2
exit 1
