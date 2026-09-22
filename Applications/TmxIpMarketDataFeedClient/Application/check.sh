#!/bin/bash
PREFIX="tmx_ip"
PID_FILE="pid.lock"
SHOW_RUNNING=false
FORCE_REPORT=false
existing_pid=""

is_process_running() {
  local pid=$1
  if [[ ! "$pid" =~ ^[1-9][0-9]*$ ]] ||
      ! kill -0 "$pid" 2> /dev/null; then
    return 1
  fi
  local state
  state=$(ps -p "$pid" -o stat= 2> /dev/null) || return 1
  if [[ "$state" == *Z* ]]; then
    return 1
  fi
  if [[ "$(uname -s)" == "Linux" ]]; then
    local executable
    executable=$(readlink "/proc/$pid/exe" 2> /dev/null) || return 1
    local directory
    directory=$(readlink "/proc/$pid/cwd" 2> /dev/null) || return 1
    local target
    target=$(readlink -f "$APPLICATION" 2> /dev/null) || return 2
    [[ "${executable% (deleted)}" == "$target" &&
      "$directory" == "$(pwd -P)" ]]
  else
    if ! command -v lsof > /dev/null; then
      echo "Error: lsof is required to identify $APPLICATION." >&2
      return 2
    fi
    local files
    if ! files=$(lsof -a -p "$pid" -d cwd,txt -Ffn 2> /dev/null); then
      kill -0 "$pid" 2> /dev/null || return 1
      return 2
    fi
    local descriptor
    local field
    local has_executable=false
    local has_directory=false
    while IFS= read -r field; do
      case "$field" in
        f*) descriptor=${field#f} ;;
        n*)
          if [[ "$descriptor" == "cwd" && "${field#n}" -ef . ]]; then
            has_directory=true
          elif [[ "$descriptor" == "txt" ]] &&
              [[ "${field#n}" -ef "$APPLICATION" ||
                "${field#n}" == "$(pwd -P)/$APPLICATION" ]]; then
            has_executable=true
          fi
          ;;
      esac
    done <<< "$files"
    $has_executable && $has_directory
  fi
}

usage() {
  cat <<EOF
Usage: $0 [-a] [-f] [-h] [-p pid] [service]

Options:
  -a   Report only when services are running (silent otherwise)
  -f   Always report status (running or not)
  -h   Show this help
  -p   Check a specific PID for the specified service

Arguments:
  service   Check only the specified service (with or without its prefix)
            If omitted, checks ${PREFIX}_*
EOF
}

check_application() (
  dir=$1
  APPLICATION="$dir"
  cd "$dir" || exit 1
  if [[ -z "$existing_pid" && -f "$PID_FILE" ]]; then
    existing_pid=$(<"$PID_FILE")
  fi
  if [[ -n "$existing_pid" ]]; then
    status=0
    is_process_running "$existing_pid" || status=$?
    if((status == 0)); then
      if $SHOW_RUNNING || $FORCE_REPORT; then
        echo "$APPLICATION is running (pid $existing_pid)."
      fi
      exit 0
    elif((status != 1)); then
      echo "Error: Unable to identify $APPLICATION (pid $existing_pid)." >&2
      exit "$status"
    fi
  fi
  if $SHOW_RUNNING && ! $FORCE_REPORT; then
    exit 1
  fi
  echo "$APPLICATION is not running."
  exit 1
)

while getopts "afhp:" opt; do
  case "$opt" in
    a) SHOW_RUNNING=true ;;
    f) FORCE_REPORT=true ;;
    h) usage; exit 0 ;;
    p) existing_pid=$OPTARG ;;
    *) usage; exit 1 ;;
  esac
done
shift $((OPTIND - 1))
if [[ -n "$existing_pid" && -z "$1" ]]; then
  echo "Error: -p requires a service name." >&2
  exit 1
fi
if [[ $# -gt 1 || "$1" == */* ]]; then
  echo "Error: Expected at most one service name without a path." >&2
  exit 1
fi
if [[ -n "$1" ]]; then
  target_dir="$1"
  if [[ "$target_dir" != "${PREFIX}_"* ]]; then
    target_dir="${PREFIX}_$target_dir"
  fi
  if [[ ! -d "$target_dir" ]]; then
    echo "Error: Directory $target_dir does not exist." >&2
    exit 1
  fi
  check_application "$target_dir"
else
  status=0
  for dir in "${PREFIX}_"*; do
    if [[ -d "$dir" ]]; then
      result=0
      check_application "$dir" || result=$?
      if((result > status)); then
        status=$result
      fi
    fi
  done
  exit "$status"
fi
