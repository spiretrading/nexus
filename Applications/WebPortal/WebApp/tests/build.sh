#!/bin/bash
set -o errexit
set -o pipefail

PROJECTS=(
  account_directory_page_tester
  account_page_tester
  compliance_page_tester
  create_account_page_tester
  dashboard_page_tester
  entitlements_page_tester
  group_info_page_tester
  group_page_tester
  loading_page_tester
  login_page_tester
  mock
  page_not_found_page_tester
  profile_page_tester
  risk_page_tester
  scratch
)

main() {
  resolve_paths
  create_forwarding_scripts
  build_projects "$@"
}

resolve_paths() {
  SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && \
    pwd -P)"
  ROOT="$(pwd -P)"
}

create_forwarding_scripts() {
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$SCRIPT_DIR/configure.sh" configure.sh
  fi
  if [[ ! -f "build.sh" ]]; then
    ln -s "$SCRIPT_DIR/build.sh" build.sh
  fi
}

get_job_count() {
  if [[ -f "/proc/cpuinfo" ]]; then
    grep -c "processor" /proc/cpuinfo
  elif command -v sysctl > /dev/null 2>&1; then
    sysctl -n hw.ncpu
  else
    echo 4
  fi
}

build_project() {
  local project="$1"
  shift
  if [[ ! -d "$project" ]]; then
    mkdir -p "$project"
  fi
  pushd "$project" > /dev/null
  "$SCRIPT_DIR/$project/build.sh" -DD="$ROOT/../library/Dependencies" "$@" 2>&1
  popd > /dev/null
}

build_projects() {
  export -f build_project
  export SCRIPT_DIR
  export ROOT
  build_project "${PROJECTS[0]}" "$@"
  local remaining=("${PROJECTS[@]:1}")
  local jobs
  jobs="$(get_job_count)"
  parallel -j"$jobs" --no-notice build_project {} "$@" ::: "${remaining[@]}"
}

main "$@"
