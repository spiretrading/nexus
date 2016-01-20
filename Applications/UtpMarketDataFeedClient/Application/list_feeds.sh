#!/bin/bash
reset=$(tput sgr0)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
feed_title="UTP"
feed_suffix="_utp"
pattern="^\./(.*)_utp$"
echo
live_feeds=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
if [ -z "${live_feeds}" ]; then
  echo "No ${feed_title} feeds are active."
else
  for var in $live_feeds; do
    feed_substr_index=$(awk -v a="${var}" -v b="_" 'BEGIN { print index(a, b) }')
    [[ $var =~ $pattern ]]
    feed=${BASH_REMATCH[1]}
    echo "${feed_title} feed ${yellow}'${feed}'${reset} is ${green}active${reset}."
  done
fi
echo
