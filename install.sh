#!/bin/bash
username=$(echo ${SUDO_USER:-${USER}})

# Get various credentials.
echo 'Enter the password to use for your Spire MySQL user:'
read -s mysql_password
echo 'Confirm:'
read -s mysql_password2
if [ "$mysql_password" != "$mysql_password2" ]; then
  echo 'Passwords do not match.'
  exit 1
fi
echo 'Enter the password to use for Spire services:'
read -s admin_password
echo 'Confirm:'
read -s admin_password2
if [ "$admin_password" != "$admin_password2" ]; then
  echo 'Passwords do not match'
  exit 1
fi
local_interface=$(echo -n `ip addr | egrep -o "inet ([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}).*global" | egrep -o "([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" | head -1`)
echo 'Enter the IP address of the global network interface to bind to.'
echo "Default is ($local_interface)"
read global_address
if [ "$global_address" == "" ]; then
  global_address=$local_interface
fi

# Install third party dependencies.
apt-get update
apt-get install parallel gcc g++ gdb git cmake zip make mysql-server ruby python3 python3-dev python3-pip libtool m4 automake libxml2 libxml2-dev libreadline6-dev libncurses5-dev vim nodejs npm <<< Y
npm install -g n
n 9.2.0
ln -sf /usr/local/n/versions/node/9.2.0/bin/node /usr/bin/nodejs

# Checkout and install Nexus
if [ ! -d "Nexus" ]; then
  sudo -u $username git clone https://github.com/spiretrading/nexus Nexus
  if [ ! -f Nexus/configure.sh ]; then
    echo 'Failed to download Nexus.'
    exit 1
  fi
fi
pushd Nexus
sudo -u $username ./configure.sh
sudo -u $username ./build.sh
popd

# Install all of the servers and clients into the home directory.
if [ ! -d "AdminClient" ]; then
  sudo -u $username mkdir AdminClient
  pushd AdminClient
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/AdminClient/Application/AdminClient .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/AdminClient/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "AdministrationServer" ]; then
  sudo -u $username mkdir AdministrationServer
  pushd AdministrationServer
  sudo -u $username cp ../Nexus/Applications/AdministrationServer/Application/AdministrationServer .
  sudo -u $username cp ../Nexus/Applications/AdministrationServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/AdministrationServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "ChartingServer" ]; then
  sudo -u $username mkdir ChartingServer
  pushd ChartingServer
  sudo -u $username cp ../Nexus/Applications/ChartingServer/Application/ChartingServer .
  sudo -u $username cp ../Nexus/Applications/ChartingServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/ChartingServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "ComplianceServer" ]; then
  sudo -u $username mkdir ComplianceServer
  pushd ComplianceServer
  sudo -u $username cp ../Nexus/Applications/ComplianceServer/Application/ComplianceServer .
  sudo -u $username cp ../Nexus/Applications/ComplianceServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/ComplianceServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "DefinitionsServer" ]; then
  sudo -u $username mkdir DefinitionsServer
  pushd DefinitionsServer
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/DefinitionsServer .
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/config.default.yml config.yml
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/countries.default.yml countries.yml
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/currencies.default.yml currencies.yml
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/date_time_zonespec.csv .
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/destinations.default.yml destinations.yml
  sudo -u $username cp ../Nexus/Applications/DefinitionsServer/Application/markets.default.yml markets.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "MarketDataServer" ]; then
  sudo -u $username mkdir MarketDataServer
  pushd MarketDataServer
  sudo -u $username cp ../Nexus/Applications/MarketDataServer/Application/MarketDataServer .
  sudo -u $username cp ../Nexus/Applications/MarketDataServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/MarketDataServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "MarketDataRelayServer" ]; then
  sudo -u $username mkdir MarketDataRelayServer
  pushd MarketDataRelayServer
  sudo -u $username cp ../Nexus/Applications/MarketDataRelayServer/Application/MarketDataRelayServer .
  sudo -u $username cp ../Nexus/Applications/MarketDataRelayServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/MarketDataRelayServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "RegistryServer" ]; then
  sudo -u $username mkdir RegistryServer
  pushd RegistryServer
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/RegistryServer/Application/RegistryServer .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/RegistryServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/RegistryServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "ReplayMarketDataFeedClient" ]; then
  sudo -u $username mkdir ReplayMarketDataFeedClient
  pushd ReplayMarketDataFeedClient
  sudo -u $username cp ../Nexus/Applications/ReplayMarketDataFeedClient/Application/ReplayMarketDataFeedClient .
  sudo -u $username cp ../Nexus/Applications/ReplayMarketDataFeedClient/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/ReplayMarketDataFeedClient/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "RiskServer" ]; then
  sudo -u $username mkdir RiskServer
  pushd RiskServer
  sudo -u $username cp ../Nexus/Applications/RiskServer/Application/RiskServer .
  sudo -u $username cp ../Nexus/Applications/RiskServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/RiskServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "ServiceLocator" ]; then
  sudo -u $username mkdir ServiceLocator
  pushd ServiceLocator
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/ServiceLocator/Application/ServiceLocator .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/ServiceLocator/Application/*.sh .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/ServiceLocator/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "SimulationMarketDataFeedClient" ]; then
  sudo -u $username mkdir SimulationMarketDataFeedClient
  pushd SimulationMarketDataFeedClient
  sudo -u $username cp ../Nexus/Applications/SimulationMarketDataFeedClient/Application/SimulationMarketDataFeedClient .
  sudo -u $username cp ../Nexus/Applications/SimulationMarketDataFeedClient/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/SimulationMarketDataFeedClient/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "SimulationOrderExecutionServer" ]; then
  sudo -u $username mkdir SimulationOrderExecutionServer
  pushd SimulationOrderExecutionServer
  sudo -u $username cp ../Nexus/Applications/SimulationOrderExecutionServer/Application/SimulationOrderExecutionServer .
  sudo -u $username cp ../Nexus/Applications/SimulationOrderExecutionServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Applications/SimulationOrderExecutionServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "UidServer" ]; then
  sudo -u $username mkdir UidServer
  pushd UidServer
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/UidServer/Application/UidServer .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/UidServer/Application/*.sh .
  sudo -u $username cp ../Nexus/Dependencies/Beam/Applications/UidServer/Application/config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi
if [ ! -d "WebPortal" ]; then
  sudo -u $username mkdir WebPortal
  pushd WebPortal
  sudo -u $username cp -R ../Nexus/Applications/WebPortal/Application/* .
  sudo -u $username mv config.default.yml config.yml
  sudo -u $username sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $username sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $username sed -i "s:admin_password:\"$admin_password\":g" config.yml
  popd
fi

# Setup the database.
mysql_input="CREATE DATABASE spire;
GRANT ALL PRIVILEGES ON spire.* TO 'spireadmin'@'localhost' IDENTIFIED BY '$mysql_password';
GRANT ALL PRIVILEGES ON spire.* TO 'spireadmin'@'%' IDENTIFIED BY '$mysql_password';
GRANT SELECT ON spire.* TO 'spireuser'@'localhost' IDENTIFIED BY '$mysql_password';
GRANT SELECT ON spire.* TO 'spireuser'@'%' IDENTIFIED BY '$mysql_password';
CREATE DATABASE market_data;
GRANT ALL PRIVILEGES ON market_data.* TO 'spireadmin'@'localhost' IDENTIFIED BY '$mysql_password';
GRANT ALL PRIVILEGES ON market_data.* TO 'spireadmin'@'%' IDENTIFIED BY '$mysql_password';
GRANT SELECT ON market_data.* TO 'spireuser'@'localhost' IDENTIFIED BY '$mysql_password';
GRANT SELECT ON market_data.* TO 'spireuser'@'%' IDENTIFIED BY '$mysql_password';
exit
"

mysql -uroot -p$mysql_password <<< "$mysql_input"

# Modify the network parameters.
if [[ $(egrep "root soft core" < /etc/security/limits.conf | wc -c) -eq 0 ]]; then
  printf 'root soft core unlimited\n' >> /etc/security/limits.conf
fi
if [[ $(egrep "* soft core" < /etc/security/limits.conf | wc -c) -eq 0 ]]; then
  printf '* soft core unlimited\n' >> /etc/security/limits.conf
fi
if [[ $(egrep "root soft nofile" < /etc/security/limits.conf | wc -c) -eq 0 ]]; then
  printf 'root soft nofile unlimited\n' >> /etc/security/limits.conf
fi
if [[ $(egrep "* soft nofile" < /etc/security/limits.conf | wc -c) -eq 0 ]]; then
  printf '* soft nofile unlimited\n' >> /etc/security/limits.conf
fi

if [[ $(egrep "net.core.rmem_max" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.core.rmem_max=16777216\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.core.wmem_max" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.core.wmem_max=16777216\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.ipv4.tcp_rmem" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.ipv4.tcp_rmem=4096 87380 16777216\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.ipv4.tcp_wmem" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.ipv4.tcp_wmem=4096 65536 16777216\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.core.netdev_max_backlog" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.core.netdev_max_backlog=30000\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.ipv4.tcp_congestion_control" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.ipv4.tcp_congestion_control=htcp\n' >> /etc/sysctl.conf
fi
if [[ $(egrep "net.ipv4.tcp_mtu_probing" < /etc/sysctl.conf | wc -c) -eq 0 ]]; then
  printf 'net.ipv4.tcp_mtu_probing=1\n' >> /etc/sysctl.conf
fi

pushd ServiceLocator
sudo -u $username ./start_server.sh
sleep 10
popd

# Add the minimum number of accounts needed to run Spire.
admin_input="mkdir administrators
mkdir services
mkdir trading_groups
cd services
mkacc uid_service $admin_password
mkacc market_data_service $admin_password
mkacc market_data_relay_service $admin_password
mkacc market_data_feed $admin_password
mkacc charting_service $admin_password
mkacc compliance_service $admin_password
mkacc order_execution_service $admin_password
mkacc risk_service $admin_password
mkacc administration_service $admin_password
mkacc definitions_service $admin_password
mkacc registry_service $admin_password
mkacc web_portal_service $admin_password
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
associate risk_service
associate order_execution_service
associate compliance_service
associate web_portal_service
exit
"

pushd AdminClient
sudo -u $username ./AdminClient <<< "$admin_input"
popd

pushd ServiceLocator
sudo -u $username ./stop_server.sh
popd

sudo -u $username cp ../Nexus/Applications/*.sh .
python_directory=$(sudo -u $username python3 -m site --user-site)
sudo -u $username cp ../Nexus/Dependencies/Beam/Beam/Library/Release/beam.so $python_directory
sudo -u $username cp ../Nexus/Nexus/Library/Release/nexus.so $python_directory
ldconfig
