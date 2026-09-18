#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_DIRECTORY=""
DEPENDENCIES=()
REPOS=()

main() {
  resolve_paths
  CACHE_DIRECTORY="$ROOT/cache_files/nexus"
  mkdir -p "$CACHE_DIRECTORY" || return 1
  add_repo "Beam" \
    "https://www.github.com/spiretrading/beam" \
    "2443ab1beae23c4db85ad17ec27df151eea272ae" 1 \
    "build_beam"
  add_dependency "lua-5.5.0" \
    "https://www.lua.org/ftp/lua-5.5.0.tar.gz" \
    "57ccc32bbbd005cab75bcc52444052535af691789dba2b9016d5c50640d68b3d" 1 \
    "build_lua"
  local quickfix_url="https://github.com/quickfix/quickfix/archive"
  local quickfix_commit="2ce8a60667d95a55cdc57a210f165e19cb757126"
  add_dependency "quickfix-v.1.16.0" \
    "$quickfix_url/$quickfix_commit.zip" \
    "b6fcea5402b443e71c751132938b8ef83efcd0167e005f5bbab103b1875614d1" 1 \
    "build_quickfix"
  add_dependency "hat-trie-0.7.0" \
    "https://github.com/Tessil/hat-trie/archive/refs/tags/v0.7.0.zip" \
    "8ea5441c06fd5d9de1ec8725bf762025a63f931949b9f49d211ab76a75ced68f" 1
  install_repos || return 1
  install_dependencies || return 1
  install_gitpython || return 1
}

build_beam() {
  ./build.sh Debug -DD="$ROOT" || return 1
  ./build.sh Release -DD="$ROOT" || return 1
}

build_lua() {
  local cores platform
  cores=$(get_core_count)
  case "$(uname -s)" in
    Darwin) platform=macosx ;;
    Linux) platform=linux ;;
    *) platform=posix ;;
  esac
  make -j "$cores" "$platform" MYCFLAGS=-fPIC || return 1
  make local || return 1
}

build_quickfix() {
  local cores
  cores=$(get_core_count)
  cmake --fresh -S . -B build -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_INSTALL_PREFIX="$PWD" -DQUICKFIX_SHARED_LIBS=OFF \
    -DQUICKFIX_EXAMPLES=OFF -DQUICKFIX_TESTS=OFF || return 1
  cmake --build build --target quickfix --parallel "$cores" || return 1
  cmake --install build || return 1
}

install_gitpython() {
  if ! python3 -c "import git" 2>/dev/null; then
    python3 -m pip install --user --break-system-packages --quiet GitPython ||
      return 1
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

add_dependency() {
  local name="$1"
  local url="$2"
  local hash="$3"
  local revision="$4"
  local build="${5:-}"
  DEPENDENCIES+=("$name|$url|$hash|$revision|$build")
}

add_repo() {
  local name="$1"
  local url="$2"
  local commit="$3"
  local revision="$4"
  local build="${5:-}"
  REPOS+=("$name|$url|$commit|$revision|$build")
}

install_dependencies() {
  for dep in "${DEPENDENCIES[@]}"; do
    IFS='|' read -r name url hash revision build <<< "$dep"
    download_and_extract "$name" "$url" "$hash" "$revision" "$build" ||
      return 1
  done
}

install_repos() {
  for repo in "${REPOS[@]}"; do
    IFS='|' read -r name url commit revision build <<< "$repo"
    clone_or_update_repo "$name" "$url" "$commit" "$revision" "$build" ||
      return 1
  done
}

download_and_extract() {
  local folder="$1"
  local build_marker="$CACHE_DIRECTORY/$folder.build_complete"
  local url="$2"
  local expected_hash="$3"
  local build_hash="$expected_hash posix-$4"
  local build_func="$5"
  local archive="${url##*/}"
  if [[ -d "$folder" && -f "$build_marker" ]] &&
      [[ "$(< "$build_marker")" == "$build_hash" ]]; then
    return 0
  fi
  rm -f "$build_marker" || return 1
  if [[ ! -f "$folder/.nexus_extract_complete" ]] ||
      [[ "$(< "$folder/.nexus_extract_complete")" != "$expected_hash" ]]; then
    rm -f "$folder/.nexus_extract_complete" || return 1
    if [[ ! -f "$archive" ]]; then
      curl -fsSL -o "$archive" "$url" || {
        rm -f "$archive"
        return 1
      }
    fi
    local actual_hash
    actual_hash=$(sha256 "$archive") || return 1
    if [[ "$actual_hash" != "$expected_hash" ]]; then
      echo "Error: SHA256 mismatch for $archive."
      rm -f "$archive"
      return 1
    fi
    mkdir -p "$folder" || return 1
    if [[ "$archive" == *.zip ]]; then
      local archive_directory
      archive_directory=$(unzip -Z -1 "$archive" | sed -n '1s,/.*,,p') ||
        return 1
      if [[ -z "$archive_directory" || "$archive_directory" == "." ||
          "$archive_directory" == ".." ]]; then
        echo "Error: Invalid archive directory."
        return 1
      fi
      unzip -qo "$archive" -d "$folder" || return 1
      cp -R "$folder/$archive_directory/." "$folder/" || return 1
      rm -r "$folder/$archive_directory" || return 1
    else
      tar -xf "$archive" --strip-components=1 -C "$folder" || return 1
    fi
    echo "$expected_hash" > "$folder/.nexus_extract_complete" || return 1
  fi
  if [[ -n "$build_func" ]]; then
    pushd "$folder" > /dev/null || return 1
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
  echo "$build_hash" > "$build_marker" || return 1
  if [[ -f "$archive" ]]; then
    rm -f "$archive" || return 1
  fi
}

clone_or_update_repo() {
  local repo_name="$1"
  local build_marker="$CACHE_DIRECTORY/$repo_name.build_complete"
  local repo_url="$2"
  local repo_commit="$3"
  local revision="$4"
  local build_func="$5"
  local is_new_repo=0
  if [[ ! -d "$repo_name" ]]; then
    rm -f "$build_marker" || return 1
    git clone "$repo_url" "$repo_name" || return 1
    is_new_repo=1
  fi
  pushd "$repo_name" > /dev/null || return 1
  if [[ "$is_new_repo" -eq 1 ]]; then
    git checkout "$repo_commit" || { popd > /dev/null; return 1; }
  fi
  if ! git merge-base --is-ancestor "$repo_commit" HEAD; then
    git fetch origin || { popd > /dev/null; return 1; }
    rm -f "$build_marker" || { popd > /dev/null; return 1; }
    git checkout "$repo_commit" || { popd > /dev/null; return 1; }
  fi
  local repo_head
  repo_head=$(git rev-parse HEAD) || { popd > /dev/null; return 1; }
  local build_hash="$repo_head posix-$revision"
  if [[ ! -f "$build_marker" ]] ||
      [[ "$(< "$build_marker")" != "$build_hash" ]]; then
    rm -f "$build_marker" || { popd > /dev/null; return 1; }
    if [[ -n "$build_func" ]]; then
      $build_func || { popd > /dev/null; return 1; }
    fi
    echo "$build_hash" > "$build_marker" ||
      { popd > /dev/null; return 1; }
  else
    (cd "$ROOT" && "./$repo_name/Beam/setup.sh") ||
      { popd > /dev/null; return 1; }
  fi
  popd > /dev/null
}

main "$@"
