#!/bin/bash
reset=$(tput sgr0)
yellow=$(tput setaf 3)
all_feeds=$(ls -d */)
feed_title="CHIX ASX"
feed_prefix="chia_"
feed_suffix="_chia"
pattern="^chia_(.*)/$"
client_app=ChiaMarketDataFeedClient
echo
for var in $all_feeds; do
  if [[ $var =~ $pattern ]];
  then
    cd $var
    feed_name=${BASH_REMATCH[1]}
    symlink=$feed_name$feed_suffix
    rm -rf "${symlink}"
    ln -s ../$client_app "${symlink}"
    echo "Softlink for ${feed_title} feed ${yellow}'${feed_name}'${reset} created."
    cd ..
  fi
done
echo
