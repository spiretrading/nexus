#!/bin/bash
beam_commit="9e2b013020de342e10d40018a47ef350a8dd57a9"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/spiretrading/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
