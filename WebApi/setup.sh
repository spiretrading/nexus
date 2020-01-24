#!/bin/bash
beam_commit="1308e6bda2a2c11a0ccf9c620e4555b2190e0843"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
