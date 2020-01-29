#!/bin/bash
reset=$(tput sgr0)
yellow=$(tput setaf 3)
all_feeds=$(ls -d */)
feed_title="JPX"
feed_prefix="jpx_"
feed_suffix="_jpx"
pattern="^jpx_(.*)/$"
client_app=JpxFlexMarketDataFeedClient
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
