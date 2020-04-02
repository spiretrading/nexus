#!/bin/bash
beam_commit="404ff39690eda6314dc0bc4d41954d32a3a3d5f1"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/spiretrading/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
