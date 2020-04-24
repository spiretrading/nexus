#!/bin/bash
beam_commit="cfb05f2182b9cfb7314e9f164b2ff58611aee9cd"
if [ ! -d "Beam" ]; then
  git clone https://www.github.com/spiretrading/beam.git Beam
fi
pushd Beam
if ! git merge-base --is-ancestor "$beam_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$beam_commit"
fi
