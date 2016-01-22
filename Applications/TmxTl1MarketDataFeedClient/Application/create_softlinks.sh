#!/bin/bash
reset=$(tput sgr0)
yellow=$(tput setaf 3)
all_feeds=$(ls -d */)
feed_title="TMX TL1"
feed_prefix="tmxtl1_"
feed_suffix="_tmxtl1"
pattern="^tmxtl1_(.*)/$"
client_app=TmxTl1MarketDataFeedClient
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
