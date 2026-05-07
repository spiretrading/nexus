#!/bin/bash
set -e
NODE_VERSION="24.15.0"
ARCH="$(uname -m)"

function print_usage() {
  echo "Usage: install.sh -p [-u] [-m] [-i] [-h]"
  echo "  -p: The password to create or use for Spire services."
  echo "  -u: The MySQL username to create or use (default is spireadmin)."
  echo "  -m: The MySQL password to create or use."
  echo "      By default it's set to the Spire password."
  echo "  -i: The global network interface to bind to."
  echo "      The default value is ($local_interface)."
}

function install_node() {
  local node_arch="$ARCH"
  if [ "$node_arch" == "x86_64" ]; then
    node_arch="x64"
  elif [[ "$node_arch" == "aarch64" || "$node_arch" == "arm64" ]]; then
    node_arch="arm64"
  else
    echo "Unsupported architecture: $node_arch"
    exit 1
  fi
  local node_dir="node-v$NODE_VERSION-linux-$node_arch"
  curl -O "https://nodejs.org/dist/v$NODE_VERSION/$node_dir.tar.xz"
  tar -xf "$node_dir.tar.xz"
  sudo cp -r "$node_dir"/{bin,include,lib,share} /usr/local/
  rm -rf "$node_dir" "$node_dir.tar.xz"
}

function check_and_install_node() {
  if command -v node &> /dev/null; then
    INSTALLED_VERSION=$(node -v | sed 's/v//')
    if [ "$INSTALLED_VERSION" = "$NODE_VERSION" ]; then
      return
    fi
  fi
  install_node
}

function install_dependencies() {
  if [ $is_root -eq 1 ]; then
    apt-get update
    apt-get install -y automake build-essential cmake curl gdb git \
      libncurses5-dev libreadline6-dev libtool libxml2 libxml2-dev m4 \
      mysql-server parallel python3 python3-dev python3-pip ruby zip
    if ! command -v yq &> /dev/null; then
      local yq_arch="$ARCH"
      if [ "$yq_arch" == "x86_64" ]; then
        yq_arch="amd64"
      elif [[ "$yq_arch" == "aarch64" || "$yq_arch" == "arm64" ]]; then
        yq_arch="arm64"
      fi
      curl -L "https://github.com/mikefarah/yq/releases/latest/download/yq_linux_$yq_arch" \
        -o /usr/local/bin/yq && chmod +x /usr/local/bin/yq
    fi
    check_and_install_node
  fi
}

if [ "$EUID" == "0" ]; then
  is_root=1
else
  is_root=0
fi
username="${SUDO_USER:-$USER}"
local_interface=$(echo -n `ip addr | \
  egrep -o "inet ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}).*global" | \
  egrep -o "([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" | \
  head -1`)
while getopts "u:m:p:i:h" opt; do
  case ${opt} in
    p)
      spire_password="$OPTARG"
      ;;
    u)
      mysql_username="$OPTARG"
      ;;
    m)
      mysql_password="$OPTARG"
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
if [ "$spire_password" == "" ] && [ "$mysql_password" == "" ]; then
  echo "Missing argument -p."
  print_usage
  exit 1
fi
if [ "$spire_password" == "" ]; then
  spire_password="$mysql_password"
fi
if [ "$mysql_username" == "" ]; then
  mysql_username="spireadmin"
fi
if [ "$mysql_password" == "" ]; then
  mysql_password="$spire_password"
fi
if [ "$global_interface" == "" ]; then
  global_interface="$local_interface"
fi
install_dependencies
sudo -u $username ./build.sh || { echo "Build failed."; exit 1; }
mysql_input="
CREATE DATABASE IF NOT EXISTS spire;
CREATE USER IF NOT EXISTS '$mysql_username'@'localhost' IDENTIFIED WITH mysql_native_password BY '$mysql_password';
ALTER USER '$mysql_username'@'localhost' IDENTIFIED WITH mysql_native_password BY '$mysql_password';
GRANT ALL ON spire.* TO '$mysql_username'@'localhost';
"
if [ $is_root -eq 1 ]; then
  mysql -uroot <<< "$mysql_input"
else
  sudo -u $username mysql -u$mysql_username -p$mysql_password <<< "$mysql_input"
fi
pushd Applications
sudo -u $username python3 setup.py -l "$local_interface" \
  -w "$global_interface" -a "$local_interface:20000" -p "$spire_password" \
  -mu "$mysql_username" -mp "$mysql_password"
sudo -u $username ./install_python.sh
pushd ServiceLocator/Application
sudo -u $username ./start.sh
sleep 10
popd
admin_input="
mkdir administrators
mkdir services
mkdir trading_groups
cd services
mkacc administration_service $spire_password
mkacc charting_service $spire_password
mkacc compliance_service $spire_password
mkacc definitions_service $spire_password
mkacc market_data_feed $spire_password
mkacc market_data_relay_service $spire_password
mkacc market_data_service $spire_password
mkacc order_execution_service $spire_password
mkacc risk_service $spire_password
mkacc uid_service $spire_password
mkacc web_portal_service $spire_password
chmod administration_service @0 7
chmod charting_service @0 1
chmod compliance_service @0 7
chmod market_data_relay_service @0 1
chmod market_data_service @0 1
chmod order_execution_service @0 7
chmod risk_service @0 7
chmod web_portal_service @0 7
cd @0
cd administrators
associate administration_service
associate compliance_service
associate market_data_relay_service
associate market_data_service
associate order_execution_service
associate risk_service
associate web_portal_service
del @1
exit
"
pushd ../Nexus/Dependencies/Beam/Applications/AdminClient/Application
sudo -u $username python3 setup.py -a "$local_interface:20000" -u "root" -p ""
sudo -u $username ./AdminClient <<< "$admin_input"
sudo -u $username python3 setup.py -a "$local_interface:20000" \
  -u "administration_service" -p "$spire_password"
popd
pushd ServiceLocator/Application
sudo -u $username ./stop.sh
popd
popd
