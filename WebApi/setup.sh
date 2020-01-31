#!/bin/bash
beam_commit="eb945d1874bf9ea88148d747b67393f11057a21e"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
