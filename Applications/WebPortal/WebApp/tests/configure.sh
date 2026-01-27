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
  configure_projects "$@"
  exit "$EXIT_STATUS"
}

resolve_paths() {
  SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && \
    pwd -P)"
  ROOT="$(pwd -P)"
  EXIT_STATUS=0
}

create_forwarding_scripts() {
  if [[ ! -f "build.sh" ]]; then
    ln -s "$SCRIPT_DIR/build.sh" build.sh
  fi
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$SCRIPT_DIR/configure.sh" configure.sh
  fi
}

configure_projects() {
  for project in "${PROJECTS[@]}"; do
    configure_project "$project" "$@"
  done
}

configure_project() {
  local project="$1"
  shift
  if [[ ! -d "$project" ]]; then
    mkdir "$project"
  fi
  pushd "$project" > /dev/null
  "$SCRIPT_DIR/$project/configure.sh" \
    -DD="$ROOT/../library/Dependencies" "$@" || EXIT_STATUS=1
  popd > /dev/null
}

main "$@"
