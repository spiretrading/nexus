#!/bin/bash
beam_commit="1105a4e3968dce3328a3b9b6d6b7d313f7b9dd14"
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
