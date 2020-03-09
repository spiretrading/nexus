#!/bin/bash
beam_commit="c5f2491aaf245d7cad90cd6f05c1294f9906bd1d"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/eidolonsystems/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
