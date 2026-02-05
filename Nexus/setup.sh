#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_NAME=""
SETUP_HASH=""
DEPENDENCIES=()
REPOS=()

main() {
  resolve_paths
  check_cache "nexus" || exit 0
  add_repo "Beam" \
    "https://www.github.com/spiretrading/beam" \
    "b98be1cc954b3b53ab251e91ca38cc4d543aee48" \
    "build_beam"
  add_dependency "lua-5.5.0" \
    "https://www.lua.org/ftp/lua-5.5.0.tar.gz" \
    "57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d" \
    "build_lua"
  add_dependency "quickfix-v.1.15.1" \
    "https://github.com/quickfix/quickfix/archive/49b3508e48f0bbafbab13b68be72250bdd971ac2.zip" \
    "0bed2ae8359fc807f351fd2d08cec13b472d27943460f1d8f0869ed8cc8c2735" \
    "build_quickfix"
  add_dependency "hat-trie-0.7.0" \
    "https://github.com/Tessil/hat-trie/archive/refs/tags/v0.7.0.zip" \
    "8ea5441c06fd5d9de1ec8725bf762025a63f931949b9f49d211ab76a75ced68f"
  install_repos || return 1
  install_dependencies || return 1
  install_gitpython
  commit
}

build_beam() {
  ./build.sh Debug -DD="$ROOT" || return 1
  ./build.sh Release -DD="$ROOT" || return 1
}

build_lua() {
  local cores
  cores=$(get_core_count)
  make -j "$cores" linux || return 1
  make local || return 1
}

build_quickfix() {
  pushd src/C++ > /dev/null
  sed -i '105s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' \
    Utility.h
  sed -i '108s/.*/template<typename T> using SmartPtr = std::shared_ptr<T>;/' \
    Utility.h
  popd > /dev/null
  ./bootstrap || return 1
  ./configure --enable-shared=no --enable-static=yes || return 1
  local cores
  cores=$(get_core_count)
  make -j "$cores" || return 1
}

install_gitpython() {
  if ! python3 -c "import git" 2>/dev/null; then
    pip3 install --user --break-system-packages GitPython || true
  fi
}

sha256() {
  if command -v sha256sum >/dev/null; then
    sha256sum "$1" | cut -d" " -f1
  else
    shasum -a 256 "$1" | cut -d" " -f1
  fi
}

get_core_count() {
  nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
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

add_dependency() {
  local name="$1"
  local url="$2"
  local hash="$3"
  local build="${4:-}"
  DEPENDENCIES+=("$name|$url|$hash|$build")
}

add_repo() {
  local name="$1"
  local url="$2"
  local commit="$3"
  local build="${4:-}"
  REPOS+=("$name|$url|$commit|$build")
}

install_dependencies() {
  for dep in "${DEPENDENCIES[@]}"; do
    IFS='|' read -r name url hash build <<< "$dep"
    download_and_extract "$name" "$url" "$hash" "$build" || return 1
  done
}

install_repos() {
  for repo in "${REPOS[@]}"; do
    IFS='|' read -r name url commit build <<< "$repo"
    clone_or_update_repo "$name" "$url" "$commit" "$build" || return 1
  done
}

download_and_extract() {
  local folder="$1"
  local url="$2"
  local expected_hash="$3"
  local build_func="$4"
  local archive="${url##*/}"
  if [[ -d "$folder" ]]; then
    return 0
  fi
  if [[ ! -f "$archive" ]]; then
    curl -fsSL -o "$archive" "$url" || return 1
  fi
  local actual_hash
  actual_hash=$(sha256 "$archive")
  if [[ "$actual_hash" != "$expected_hash" ]]; then
    echo "Error: SHA256 mismatch for $archive."
    echo "  Expected: $expected_hash"
    echo "  Actual:   $actual_hash"
    rm -f "$archive"
    return 1
  fi
  mkdir -p "$folder" || return 1
  if [[ "$archive" == *.zip ]]; then
    unzip -q "$archive" -d "$folder" || { rm -rf "$folder"; return 1; }
  else
    tar -xf "$archive" -C "$folder" || { rm -rf "$folder"; return 1; }
  fi
  flatten_directory "$folder"
  if [[ -n "$build_func" ]]; then
    pushd "$folder" > /dev/null
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
  rm -f "$archive"
}

flatten_directory() {
  local folder="$1"
  local dir_count=0
  local file_count=0
  local single_dir=""
  for d in "$folder"/*/; do
    if [[ -d "$d" ]]; then
      ((dir_count += 1))
      single_dir="$d"
    fi
  done
  for f in "$folder"/*; do
    if [[ -f "$f" ]]; then
      ((file_count += 1))
    fi
  done
  if [[ "$dir_count" -eq 1 ]] && [[ "$file_count" -eq 0 ]]; then
    shopt -s dotglob
    mv "$single_dir"* "$folder/" 2>/dev/null || true
    shopt -u dotglob
    rmdir "$single_dir" 2>/dev/null || true
  fi
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

main "$@"
