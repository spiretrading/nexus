#!/bin/bash
beam_commit="0f0ebec0c9d5c2810bb8e8733b9bc5c903f9e377"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
