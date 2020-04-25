#!/bin/bash
function print_usage() {
  echo "Usage: install.sh -m [-u] [-p] [-i] [-h]"
  echo "  -m: The MySQL password to create or use."
  echo "  -u: The MySQL username to create or use (default is spireadmin)."
  echo "  -p: The password to create or use for Spire services."
  echo "      By default it's set to the MySQL password."
  echo "  -i: The global network interface to bind to."
  echo "      The default value is ($local_interface)."
}

function install_dependencies() {
  if [ $is_root -eq 1 ]; then
    apt-get update
    apt-get install -y automake cmake g++ gcc gdb git libncurses5-dev \
      libreadline6-dev libtool libxml2 libxml2-dev m4 make mysql-server nodejs \
      npm parallel python3 python3-dev python3-pip ruby zip
  fi
}

if [ "$EUID" == "0" ]; then
  is_root=1
else
  is_root=0
fi
username=$(echo ${SUDO_USER:-${USER}})
local_interface=$(echo -n `ip addr | \
  grep -o "inet ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}).*global" | \
  grep -o "([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" | \
  head -1`)

while getopts "u:m:p:i:h" opt; do
  case ${opt} in
    m)
      mysql_password="$OPTARG"
      ;;
    u)
      mysql_username="$OPTARG"
      ;;
    p)
      spire_password="$OPTARG"
      ;;
    i)
      global_interface="$OPTARG"
      ;;
    h)
      print_usage
      exit 0
      ;;
    \? )
      print_usage
      exit 1
      ;;
  esac
done
if [ "$mysql_password" == "" ]; then
  echo "Missing argument -m for MySQL password."
  print_usage
  exit 1
fi
if [ "$mysql_username" == "" ]; then
  mysql_username="spireadmin"
fi
if [ "$spire_password" == "" ]; then
  spire_password="$mysql_password"
fi
if [ "$global_interface" == "" ]; then
  global_interface="$local_interface"
fi
install_dependencies
sudo -u $username ./configure.sh
sudo -u $username ./build.sh

mysql_input="
CREATE DATABASE spire;
GRANT ALL PRIVILEGES ON spire.* TO '$mysql_username'@'localhost' IDENTIFIED BY
  '$mysql_password';
CREATE DATABASE market_data;
GRANT ALL PRIVILEGES ON market_data.* TO '$mysql_username'@'localhost'
  IDENTIFIED BY '$mysql_password';
exit
"
if [ $is_root -eq 1 ]; then
  mysql -uroot <<< "$mysql_input"
else
  sudo -u mysql -u$mysql_username -p$mysql_password <<< "$mysql_input"
fi
pushd Applications
sudo -u $username python3 setup.py -l "$local_interface" -w "global_interface" \
  -a "$local_interface" -p "$spire_password" -ma "$local_interface" \
  -mu "$mysql_username" -mp "$mysql_password"
sudo -u $username ./install_python.sh
popd
