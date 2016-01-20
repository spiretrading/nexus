#!/bin/bash
reset=$(tput sgr0)
red=$(tput setaf 1)
yellow=$(tput setaf 3)
feed_title="CHIX ASX"
feed_prefix="chia_"
feed_suffix="_chia"
pattern="^\./(.*)${feed_suffix}$"
echo
if [ "$1" = "" ]; then
  echo "Invalid parameters supplied."
  echo
  echo "Usage: ./stop_feed.sh <feed_name> Stops a single ${feed_title} feed."
  echo "       ./stop_feed.sh all         Stops all ${feed_title} feeds."
  echo
else
  if [ "$1" = "all" ]; then
    # User has chosen to terminate all feeds.
    feed_pids=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{ print $2 }')
    live_feeds=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{ print $8 }')
    if [ -z "${feed_pids}" ]; then
      echo "${red}[ERROR]${reset} No ${feed_title} feeds are active."
      echo "        Operation aborted."
      echo
    else
      # Terminate all active feeds.
      for command in $live_feeds; do
        var=$(ps -ef | grep -i "${command}" | grep -v "grep" | grep -v "bash" | awk '{ print $2 }')
        [[ $command =~ $pattern ]]
        feed_name=${BASH_REMATCH[1]}
        feed_dir=$feed_prefix$feed_name"/"
        check=$(awk -v a="${live_feeds}" -v b="${feed_name}" 'BEGIN { print index(a, b) }')
        if [ "$check" = "0" ]; then
          echo "${red}[ERROR]${reset} Could not stop ${feed_title} feed ${yellow}'${feed_name}'${reset}."
          echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is inactive."
          echo
        else
          kill -s INT $var
          echo "${feed_title} feed ${yellow}'${feed_name}'${reset} terminated."
          # Rename and move 'srv.log' files.
          log_dir=$feed_dir"logs"
          cd $feed_dir
          srv_logs=$(ls srv_*.log 2>/dev/null)
          if [ -n "${srv_logs}" ]; then
            if [ ! -d "logs" ]; then
              mkdir logs
            fi
            for srv_log in $srv_logs; do
              mv $srv_log logs
            done
          else
            echo "${red}[ERROR]${reset} No log file could be found."
          fi
          cd ..
        fi
      done
      echo
    fi
  else
    # User has chosen to terminate a single feed.
    feed_name=$1
    executable=$feed_name$feed_suffix
    all_feeds=$(ls -d ${feed_prefix}*/)
    feed_dir=$feed_prefix$feed_name"/"
    check=$(awk -v a="${all_feeds}" -v b="${feed_dir}" 'BEGIN { print index(a, b) }')
    if [ "${check}" = "0" ]; then
      # Invalid feed supplied.
      echo "${red}[ERROR]${reset} Could not stop ${feed_title} feed ${yellow}'${feed_name}'${reset}."
      echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is invalid."
      echo
    else
      check_string="./"$executable
      active_feeds=$(ps -ef | grep -i ".*${feed_suffix}" | grep -v "grep" | grep -v "bash" | awk '{print $8}')
      is_active=$(awk -v a="${active_feeds}" -v b="${check_string}" 'BEGIN { print index(a, b) }')
      if [ "${is_active}" = "0" ]; then
        # Attempting to terminate an inactive feed.
        echo "${red}[ERROR]${reset} Could not stop ${feed_title} feed ${yellow}'${feed_name}'${reset}."
        echo "        ${feed_title} feed ${yellow}'${feed_name}'${reset} is inactive."
        echo
      else
        # Terminate feed.
        pid=$(ps -ef | grep "${executable}" | grep -v "grep" | grep -v "bash" | awk '{print $2}')
        kill -s INT $pid
        echo "${feed_title} feed ${yellow}'${feed_name}'${reset} terminated."
        echo
        cd $feed_dir
        srv_logs=$(ls srv_*.log 2>/dev/null)
        if [ -n "${srv_logs}" ]; then
          if [ ! -d "logs" ]; then
            mkdir logs
          fi
          for srv_log in $srv_logs; do
            mv $srv_log logs
          done
        else
          echo "${red}[ERROR]${reset} No log file could be found."
        fi
      fi
    fi
  fi
fi
