#include <fstream>
#include <iostream>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/Accounting/Portfolio.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd("", ' ', "0.9-r\nCopyright (C) 2009 Eidolon Systems Ltd.");
    ValueArg<string> configArg("c", "config", "Configuration file", false,
      "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  ifstream configStream(configFile.c_str());
  if(!configStream.good()) {
    cerr << configFile << " not found." << endl;
    return -1;
  }
  YAML::Node config;
  try {
    YAML::Parser configParser(configStream);
    configParser.GetNextDocument(config);
  } catch(YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      *config.FindValue("service_locator"));
  } catch(std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  ApplicationServiceLocatorClient serviceLocatorClient;
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_address,
      Ref(socketThreadPool), Ref(timerThreadPool));
    serviceLocatorClient->SetCredentials(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password);
    serviceLocatorClient->Open();
  } catch(std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  ApplicationAdministrationClient administrationClient;
  try {
    administrationClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    administrationClient->Open();
  } catch(std::exception&) {
    cerr << "Unable to connect to the administration service." << endl;
    return -1;
  }
  EntitlementDatabase entitlementDatabase =
    administrationClient->LoadEntitlements();
  CurrencyDatabase currencyDatabase = definitionsClient->LoadCurrencyDatabase();
  vector<DirectoryEntry> tradingGroups =
    administrationClient->LoadManagedTradingGroups(
    serviceLocatorClient->GetAccount());
  int count = 0;
  Money cost;
  for(const DirectoryEntry& group : tradingGroups) {
    DirectoryEntry traderDirectory = serviceLocatorClient->LoadDirectoryEntry(
      group, "traders");
    vector<DirectoryEntry> traders = serviceLocatorClient->LoadChildren(
      traderDirectory);
    for(const DirectoryEntry& trader : traders) {
      AccountIdentity identity = administrationClient->LoadIdentity(trader);
      vector<DirectoryEntry> entitlements =
        administrationClient->LoadEntitlements(trader);
      if(!entitlements.empty()) {
        ++count;
      }
      for(const DirectoryEntry& entitlement : entitlements) {
        const EntitlementDatabase::Entry& entry = *std::find_if(
          entitlementDatabase.GetEntries().begin(),
          entitlementDatabase.GetEntries().end(),
          [&] (const EntitlementDatabase::Entry& e) {
            return e.m_groupEntry == entitlement;
          });
        string currencySign = currencyDatabase.FromId(
          entry.m_currency).m_sign;
        string currencyCode = currencyDatabase.FromId(
          entry.m_currency).m_code.GetData();
        cout << trader.m_name << ", " << identity.m_firstName << " " <<
          identity.m_lastName << ", " << entry.m_name << ", " <<
          currencySign << ToString(entry.m_price) << " " << currencyCode <<
          endl;
        cost += entry.m_price;
      }
    }
  }
  cout << "Total traders: " << count << endl;
  cout << "Total cost: " << ToString(cost) << endl;
  return 0;
}
