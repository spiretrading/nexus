#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_NAME=""
SETUP_HASH=""
REPOS=()

main() {
  resolve_paths
  "$DIRECTORY/../../../WebApi/setup.sh"
  check_cache "nexus_webapp" || exit 0
  add_repo "dali" \
    "https://www.github.com/spiretrading/dali" \
    "2c305bb47a518b870bf5cc27697bd27ccb9a848c"
  install_repos || return 1
  configure_webapi || return 1
  commit
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  ROOT="$(pwd -P)"
}

sha256() {
  if command -v sha256sum >/dev/null; then
    sha256sum "$1" | cut -d" " -f1
  else
    shasum -a 256 "$1" | cut -d" " -f1
  fi
}

check_cache() {
  CACHE_NAME="$1"
  SETUP_HASH=$(sha256 "$DIRECTORY/setup.sh")
  if [[ -f "cache_files/$CACHE_NAME.txt" ]]; then
    local cached_hash
    cached_hash=$(cat "cache_files/$CACHE_NAME.txt")
    if [[ "$SETUP_HASH" == "$cached_hash" ]]; then
      return 1
    fi
  fi
  return 0
}

commit() {
  if [[ ! -d "cache_files" ]]; then
    mkdir -p cache_files || return 1
  fi
  echo "$SETUP_HASH" > "cache_files/$CACHE_NAME.txt"
}

add_repo() {
  local name="$1"
  local url="$2"
  local commit="$3"
  local build="${4:-}"
  REPOS+=("$name|$url|$commit|$build")
}

install_repos() {
  for repo in "${REPOS[@]}"; do
    IFS='|' read -r name url commit build <<< "$repo"
    clone_or_update_repo "$name" "$url" "$commit" "$build" || return 1
  done
}

clone_or_update_repo() {
  local repo_name="$1"
  local repo_url="$2"
  local repo_commit="$3"
  local build_func="$4"
  local needs_build=0
  if [[ ! -d "$repo_name" ]]; then
    git clone "$repo_url" "$repo_name" || { rm -rf "$repo_name"; return 1; }
    pushd "$repo_name" > /dev/null
    git checkout "$repo_commit"
    popd > /dev/null
    needs_build=1
  else
    pushd "$repo_name" > /dev/null
    if ! git merge-base --is-ancestor "$repo_commit" HEAD; then
      git checkout master
      git pull
      git checkout "$repo_commit"
      needs_build=1
    fi
    popd > /dev/null
  fi
  if [[ "$needs_build" == "1" ]] && [[ -n "$build_func" ]]; then
    pushd "$repo_name" > /dev/null
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
}

configure_webapi() {
  if [[ -d "WebApi" ]]; then
    return 0
  fi
  mkdir -p WebApi || return 1
  pushd WebApi > /dev/null
  "$DIRECTORY/../../../WebApi/configure.sh" -DD="$ROOT" ||
    { popd > /dev/null; return 1; }
  popd > /dev/null
}

main "$@"
