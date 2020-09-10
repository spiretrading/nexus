#!/bin/bash
beam_commit="166cdd2ce55d020066bd3c5ea1df040f8d4a04dc"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/spiretrading/beam.git Beam
  if [ "$?" == "0" ]; then
    pushd Beam
    git checkout "$beam_commit"
    popd
  else
    rm -rf Beam
    exit_status=1
  fi
fi
if [ -d "Beam" ]; then
  pushd Beam
  if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
    git checkout master
    git pull
    git checkout "$beam_commit"
  fi
  popd
fi
