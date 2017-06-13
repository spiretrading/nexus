#!/bin/bash
reset=$(tput sgr0)
red=$(tput setaf 1)
yellow=$(tput setaf 3)
cyan=$(tput setaf 6)
feed_title="ASX ITCH"
feed_prefix="asxitch_"
feed_suffix="_asxitch"
pattern="^asxitch_(.*)/$"
echo
if [ "$1" = "" ]; then
  echo "Invalid parameters supplied."
  echo
  echo "Usage: ./start_feed.sh <feed_name> Starts a single ${feed_title} feed."
  echo "       ./start_feed.sh all         Starts all ${feed_title} feeds."
  echo
else
  all_feed_dirs=$(ls -d $feed_prefix*/)
  if [ "$1" = "all" ]; then
    # User has chosen to start all feeds.
    for var in $all_feed_dirs; do
      if [[ $var =~ $pattern ]];
      then
        cd $var
        feed_name=${BASH_REMATCH[1]}
        symlink=$feed_name$feed_suffix
        if [ -h "${symlink}" ]; then
          # Valid soft-link.
          live_feeds=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
          check=$(awk -v a="${live_feeds}" -v b="${feed_name}" 'BEGIN { print index(a, b) }')
          if [ "${check}" = "0" ]; then
            leftover=$(ls srv_*.log 2>/dev/null)
            if [ -n "${leftover}" ]; then
              if [ ! -d "logs" ]; then
                mkdir logs
              fi
              for var in $leftover; do
                mv $var logs
              done
            fi
            date_time=$(date '+%Y%m%d_%H_%M_%S')
            new_log_name="srv_${date_time}.log"
            ./$symlink > $new_log_name &
            echo "${feed_title} feed ${yellow}'${feed_name}'${reset} has been started."
            cd ..
          else
            # Feed is already active.
            echo "${red}[ERROR]${reset} Could not start ${feed_title} feed ${yellow}'${feed_name}'${reset}."
            echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is already active."
            echo
            cd ..
          fi
        else
          # Soft-link for feed is missing from its sub-directory.
          echo "${red}[ERROR]${reset} ${feed_title} feed ${yellow}'${feed_name}'${reset} failed to start."
          echo "        Ensure the ${feed_title} ${cyan}'${symlink}'${reset} soft-link exists."
          echo
          cd ..
        fi
      else
        # Target directory is not a valid feed sub-directory.
        echo "${red}[ERROR]${reset} Invalid ${feed_title} feed sub-directory '${var}'."
      fi
    done
    echo
  else
    # Start a single feed.
    feed_name=$1
    symlink=$feed_name$feed_suffix
    feed_dir=$feed_prefix$feed_name"/"
    check=$(awk -v a="${all_feed_dirs}" -v b="${feed_dir}" 'BEGIN { print index(a, b) }')
    if [ "${check}" = "0" ]; then
      # Feed supplied is invalid.
      echo "${red}[ERROR]${reset} Could not start ${feed_title} feed ${yellow}'${feed_name}'${reset}."
      echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is invalid."
      echo
    else
      active_feeds=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
      check=$(awk -v a="${active_feeds}" -v b="${feed_name}" 'BEGIN { print index(a, b) }')
      if [ "${check}" = "0" ]; then
        cd $feed_dir
        if [ -h "${symlink}" ]; then
          leftover=$(ls srv_*.log 2>/dev/null)
          if [ -n "${leftover}" ]; then
            if [ ! -d "logs" ]; then
              mkdir logs
            fi
            for var in $leftover; do
              mv $var logs
            done
          fi
          # Start a single feed.
          date_time=$(date '+%Y%m%d_%H_%M_%S')
          new_log_name="srv_${date_time}.log"
          ./$symlink > $new_log_name &
          echo "${feed_title} feed ${yellow}'${feed_name}'${reset} has been started."
          cd ..
          ./list_feeds.sh
        else
          # Feed soft-link does not exist.
          echo "${red}[ERROR]${reset} ${feed_title} feed ${yellow}'${feed_name}'${reset} failed to start."
          echo "        Ensure the ${feed_title} ${cyan}'${symlink}'${reset} soft-link exists."
          echo
        fi
      else
        # Feed is already active.
        echo "${red}[ERROR]${reset} Could not start ${feed_title} feed ${yellow}'${feed_name}'${reset}."
        echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is already active."
        echo
      fi
    fi
  fi
fi
