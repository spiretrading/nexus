#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "AdministrationServer/Version.hpp"
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/CachedAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/MySqlAdministrationDataStore.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::MySql;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace std;
using namespace TCLAP;

namespace {
  typedef ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
    MetaAdministrationServlet<ApplicationServiceLocatorClient::Client*,
    CachedAdministrationDataStore<MySqlAdministrationDataStore*>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>
    AdministrationServletContainer;

  struct AdministrationServerConnectionInitializer {
    string m_serviceName;
    IpAddress m_interface;
    vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void AdministrationServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<string>(config, "service",
      AdministrationService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    vector<IpAddress> addresses;
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }

  EntitlementDatabase ParseEntitlements(const YAML::Node& config,
      const CurrencyDatabase& currencies,
      ApplicationServiceLocatorClient& serviceLocatorClient) {
    auto entitlementsDirectory = LoadOrCreateDirectory(*serviceLocatorClient,
      "entitlements", DirectoryEntry::GetStarDirectory());
    EntitlementDatabase database;
    for(auto& entitlementConfig : config) {
      EntitlementDatabase::Entry entry;
      entry.m_name = Extract<string>(entitlementConfig, "name");
      auto price = Money::FromValue(Extract<string>(entitlementConfig,
        "price"));
      if(!price.is_initialized()) {
        throw runtime_error("Invalid entitlement price.");
      }
      entry.m_price = *price;
      entry.m_currency = currencies.FromCode(
        Extract<string>(entitlementConfig, "currency")).m_id;
      auto groupName = Extract<string>(entitlementConfig, "group");
      entry.m_groupEntry = LoadOrCreateDirectory(*serviceLocatorClient,
        groupName, entitlementsDirectory);
      auto& applicability = GetNode(entitlementConfig, "applicability");
      for(auto& applicabilityConfig : applicability) {
        MarketCode market = Extract<string>(applicabilityConfig, "market", "");
        MarketCode source = Extract<string>(applicabilityConfig, "source");
        EntitlementKey key{market, source};
        auto& messages = GetNode(applicabilityConfig, "messages");
        for(auto& messageConfig : messages) {
          auto message = messageConfig.to<string>();
          if(message == "BBO") {
            entry.m_applicability[key].Set(MarketDataType::BBO_QUOTE);
          } else if(message == "MQT") {
            entry.m_applicability[key].Set(MarketDataType::MARKET_QUOTE);
          } else if(message == "BOOK") {
            entry.m_applicability[key].Set(MarketDataType::BOOK_QUOTE);
          } else if(message == "TAS") {
            entry.m_applicability[key].Set(MarketDataType::TIME_AND_SALE);
          } else if(message == "IMB") {
            entry.m_applicability[key].Set(MarketDataType::ORDER_IMBALANCE);
          }
        }
      }
      database.Add(entry);
    }
    return database;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" ADMINISTRATION_SERVER_VERSION
      "\nCopyright (C) 2009 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  AdministrationServerConnectionInitializer
    administrationServerConnectionInitializer;
  try {
    administrationServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  ServiceLocatorClientConfig serviceLocatorClientConfig;
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
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
  } catch(const std::exception& e) {
    cerr << "Error logging in: " << e.what() << endl;
    return -1;
  }
  MySqlConfig mySqlConfig;
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  auto organizationName = Extract<string>(config, "organization",
    "Spire Trading Inc.");
  EntitlementDatabase entitlements;
  try {
    entitlements = ParseEntitlements(GetNode(config, "entitlements"),
      definitionsClient->LoadCurrencyDatabase(), serviceLocatorClient);
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'entitlements': " << e.what() << endl;
    return -1;
  }
  auto accountSource =
    [&] (unsigned int id) {
      return serviceLocatorClient->LoadDirectoryEntry(id);
    };
  MySqlAdministrationDataStore mySqlDataStore{mySqlConfig.m_address,
    mySqlConfig.m_schema, mySqlConfig.m_username, mySqlConfig.m_password,
    accountSource};
  AdministrationServletContainer administrationServer{
    Initialize(serviceLocatorClient.Get(),
    Initialize(serviceLocatorClient.Get(), organizationName, entitlements,
    Initialize(&mySqlDataStore))),
    Initialize(administrationServerConnectionInitializer.m_interface,
    Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>{}, seconds{10},
    Ref(timerThreadPool))};
  try {
    administrationServer.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    JsonObject administrationService;
    administrationService["addresses"] =
      ToString(administrationServerConnectionInitializer.m_addresses);
    serviceLocatorClient->Register(
      administrationServerConnectionInitializer.m_serviceName,
      administrationService);
  } catch(const std::exception& e) {
    cerr << "Error registering service: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
