#!/bin/bash

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
local_interface=$(echo -n `ifconfig -a | egrep -o "([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})" | head -1`)
echo 'Enter the IP address of the global network interface to bind to.'
echo "Default is ($local_interface)"
read global_address
if [ "$global_address" == "" ]; then
  global_address=$local_interface
fi

# Create a developers group and home directory and install all source code to it
groupadd developers
usermod -a -G developers $(logname)
cd /home
mkdir developers
chgrp developers developers 
chmod g+rwx ./developers
apt-get update
apt-get install parallel gcc g++ gdb git cmake zip make mysql-server ruby python python-dev libtool m4 automake libxml2 libxml2-dev libreadline6-dev libncurses5-dev vim nodejs nodejs-legacy npm <<< Y
npm install webpack -g
npm install -g grunt-cli

# Locate MySQL's shared library
cd /etc/ld.so.conf.d
if [[ $(egrep "/usr/local/mysql/lib/" < libc.conf | wc -c) -eq 0 ]]; then
  sed -i '$a /usr/local/mysql/lib/' libc.conf
  ldconfig
fi

# Checkout and install Beam
cd /home/developers
if [ ! -d "Beam" ]; then
  sudo -u $(logname) git clone https://github.com/eidolonsystems/beam.git Beam
  if [ ! -f "./Beam/Build/Make/setup.sh" ]; then
    echo 'Failed to download Beam.'
    exit 1
  fi
fi
./Beam/Build/Make/setup.sh
ldconfig
cd /home/developers/Beam/Build/Make
sudo -u $(logname) ./run_cmake.sh
sudo -u $(logname) ./build.sh
cd /home/developers

# Checkout and install Nexus
if [ ! -d "Nexus" ]; then
  sudo -u $(logname) git clone https://github.com/eidolonsystems/nexus.git Nexus
  if [ ! -f "./Nexus/Build/Make/setup.sh" ]; then
    echo 'Failed to download Nexus.'
    exit 1
  fi
fi
./Nexus/Build/Make/setup.sh
ldconfig
cd /home/developers/Nexus/Build/Make
sudo -u $(logname) ./run_cmake.sh
sudo -u $(logname) ./build.sh

# Install all of the servers and clients into the home directory.
cd ~/
if [ ! -d "AdminClient" ]; then
  sudo -u $(logname) mkdir AdminClient
  cd AdminClient
  sudo -u $(logname) cp /home/developers/Beam/Applications/AdminClient/Application/AdminClient .
  sudo -u $(logname) cp /home/developers/Beam/Applications/AdminClient/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "AdministrationServer" ]; then
  sudo -u $(logname) mkdir AdministrationServer
  cd AdministrationServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/AdministrationServer/Application/AdministrationServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/AdministrationServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/AdministrationServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "ChartingServer" ]; then
  sudo -u $(logname) mkdir ChartingServer
  cd ChartingServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ChartingServer/Application/ChartingServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ChartingServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ChartingServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "ClientWebPortal" ]; then
  sudo -u $(logname) mkdir ClientWebPortal
  cd ClientWebPortal
  sudo -u $(logname) cp -R /home/developers/Nexus/Applications/ClientWebPortal/Application/* .
  sudo -u $(logname) mv config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "ComplianceServer" ]; then
  sudo -u $(logname) mkdir ComplianceServer
  cd ComplianceServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ComplianceServer/Application/ComplianceServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ComplianceServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/ComplianceServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "DefinitionsServer" ]; then
  sudo -u $(logname) mkdir DefinitionsServer
  cd DefinitionsServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/DefinitionsServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/config.default.yml config.yml
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/countries.default.yml countries.yml
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/currencies.default.yml currencies.yml
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/date_time_zonespec.csv .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/destinations.default.yml destinations.yml
  sudo -u $(logname) cp /home/developers/Nexus/Applications/DefinitionsServer/Application/markets.default.yml markets.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "MarketDataServer" ]; then
  sudo -u $(logname) mkdir MarketDataServer
  cd MarketDataServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataServer/Application/MarketDataServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "MarketDataRelayServer" ]; then
  sudo -u $(logname) mkdir MarketDataRelayServer
  cd MarketDataRelayServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataRelayServer/Application/MarketDataRelayServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataRelayServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/MarketDataRelayServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "RegistryServer" ]; then
  sudo -u $(logname) mkdir RegistryServer
  cd RegistryServer
  sudo -u $(logname) cp /home/developers/Beam/Applications/RegistryServer/Application/RegistryServer .
  sudo -u $(logname) cp /home/developers/Beam/Applications/RegistryServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Beam/Applications/RegistryServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "RiskServer" ]; then
  sudo -u $(logname) mkdir RiskServer
  cd RiskServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/RiskServer/Application/RiskServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/RiskServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/RiskServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "ServiceLocator" ]; then
  sudo -u $(logname) mkdir ServiceLocator
  cd ServiceLocator
  sudo -u $(logname) cp /home/developers/Beam/Applications/ServiceLocator/Application/ServiceLocator .
  sudo -u $(logname) cp /home/developers/Beam/Applications/ServiceLocator/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Beam/Applications/ServiceLocator/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "SimulationMarketDataFeedClient" ]; then
  sudo -u $(logname) mkdir SimulationMarketDataFeedClient
  cd SimulationMarketDataFeedClient
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationMarketDataFeedClient/Application/SimulationMarketDataFeedClient .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationMarketDataFeedClient/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationMarketDataFeedClient/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "SimulationOrderExecutionServer" ]; then
  sudo -u $(logname) mkdir SimulationOrderExecutionServer
  cd SimulationOrderExecutionServer
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationOrderExecutionServer/Application/SimulationOrderExecutionServer .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationOrderExecutionServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Nexus/Applications/SimulationOrderExecutionServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
fi
if [ ! -d "UidServer" ]; then
  sudo -u $(logname) mkdir UidServer
  cd UidServer
  sudo -u $(logname) cp /home/developers/Beam/Applications/UidServer/Application/UidServer .
  sudo -u $(logname) cp /home/developers/Beam/Applications/UidServer/Application/*.sh .
  sudo -u $(logname) cp /home/developers/Beam/Applications/UidServer/Application/config.default.yml config.yml
  sudo -u $(logname) sed -i "s:local_interface:$local_interface:g" config.yml
  sudo -u $(logname) sed -i "s:global_address:$global_address:g" config.yml
  sudo -u $(logname) sed -i "s:admin_password:\"$admin_password\":g" config.yml
  cd ..
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

sudo -u $(logname) mysql -uroot -p$mysql_password <<< "$mysql_input"

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

cd ServiceLocator
sudo -u $(logname) ./start_server.sh
sleep 10
cd ..

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

cd AdminClient
sudo -u $(logname) ./AdminClient <<< "$admin_input"
cd ..

cd ServiceLocator
sudo -u $(logname) ./stop_server.sh
cd ..

check_script="#!/bin/bash
services=\"ServiceLocator\"
services+=\" UidServer\"
services+=\" RegistryServer\"
services+=\" DefinitionsServer\"
services+=\" AdministrationServer\"
services+=\" MarketDataServer\"
services+=\" MarketDataRelayServer\"
services+=\" ChartingServer\"
services+=\" ComplianceServer\"
services+=\" SimulationOrderExecutionServer\"
services+=\" RiskServer\"
services+=\" SimulationMarketDataFeedClient\"
services+=\" ClientWebPortal\"

for directory in \$services; do
  cd \$directory
  ./check_server.sh
  cd ..
done
"

if [ ! -f "check.sh" ]; then
  sudo -u $(logname) touch check.sh
  sudo -u $(logname) echo -n "$check_script" > check.sh
  sudo -u $(logname) chmod +x ./check.sh
fi

start_script="#!/bin/bash
services=\"ServiceLocator\"
services+=\" UidServer\"
services+=\" RegistryServer\"
services+=\" DefinitionsServer\"
services+=\" AdministrationServer\"
services+=\" MarketDataServer\"
services+=\" MarketDataRelayServer\"
services+=\" ChartingServer\"
services+=\" ComplianceServer\"
services+=\" SimulationOrderExecutionServer\"
services+=\" RiskServer\"
services+=\" SimulationMarketDataFeedClient\"
services+=\" ClientWebPortal\"

mysql -h 127.0.0.1 -uspireadmin -p$mysql_password -Dspire < ./reset.sql

for directory in \$services; do
  sleep 10
  cd \$directory
  ./stop_server.sh
  ./start_server.sh
  cd ..
done
"

if [ ! -f "start.sh" ]; then
  sudo -u $(logname) touch start.sh
  sudo -u $(logname) echo -n "$start_script" > start.sh
  sudo -u $(logname) chmod +x ./start.sh
fi

stop_script="#!/bin/bash
services=\"ServiceLocator\"
services+=\" UidServer\"
services+=\" RegistryServer\"
services+=\" DefinitionsServer\"
services+=\" AdministrationServer\"
services+=\" MarketDataServer\"
services+=\" MarketDataRelayServer\"
services+=\" ChartingServer\"
services+=\" ComplianceServer\"
services+=\" SimulationOrderExecutionServer\"
services+=\" RiskServer\"
services+=\" SimulationMarketDataFeedClient\"
services+=\" ClientWebPortal\"

for directory in \$services; do
  cd \$directory
  ./stop_server.sh
  cd ..
done
"

if [ ! -f "stop.sh" ]; then
  sudo -u $(logname) touch stop.sh
  sudo -u $(logname) echo -n "$stop_script" > stop.sh
  sudo -u $(logname) chmod +x ./stop.sh
fi

if [ ! -f "reset.sql" ]; then
  sudo -u $(logname) touch reset.sql
  sudo -u $(logname) echo -n "update risk_states set state = 0;" > reset.sql
fi

cp /home/developers/Beam/Beam/Library/Release/beam.so /usr/local/lib/python2.7/dist-packages/
cp /home/developers/Nexus/Nexus/Library/Release/nexus.so /usr/local/lib/python2.7/dist-packages/
ldconfig
