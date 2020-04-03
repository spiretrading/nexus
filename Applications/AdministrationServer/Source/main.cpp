#include <fstream>
#include <iostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <tclap/CmdLine.h>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/AdministrationServlet.hpp"
#include "Nexus/AdministrationService/CachedAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/SqlAdministrationDataStore.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
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
using namespace Viper;

namespace {
  using AdministrationServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaAdministrationServlet<
    ApplicationServiceLocatorClient::Client*, CachedAdministrationDataStore<
    SqlAdministrationDataStore<MySql::Connection>>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

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
    auto addresses = vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<vector<IpAddress>>(config, "addresses", addresses);
  }

  EntitlementDatabase ParseEntitlements(const YAML::Node& config,
      const CurrencyDatabase& currencies,
      ApplicationServiceLocatorClient& serviceLocatorClient) {
    auto entitlementsDirectory = LoadOrCreateDirectory(*serviceLocatorClient,
      "entitlements", DirectoryEntry::GetStarDirectory());
    auto database = EntitlementDatabase();
    for(auto entitlementConfig : config) {
      auto entry = EntitlementDatabase::Entry();
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
      auto applicability = GetNode(entitlementConfig, "applicability");
      for(auto applicabilityConfig : applicability) {
        auto market = MarketCode(
          Extract<string>(applicabilityConfig, "market", ""));
        auto source = MarketCode(
          Extract<string>(applicabilityConfig, "source"));
        auto key = EntitlementKey(market, source);
        auto messages = GetNode(applicabilityConfig, "messages");
        for(auto messageConfig : messages) {
          auto message = messageConfig.as<string>();
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
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" ADMINISTRATION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto administrationServerConnectionInitializer =
    AdministrationServerConnectionInitializer();
  try {
    administrationServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'server': " << e.what() << endl;
    return -1;
  }
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'service_locator': " << e.what() << endl;
    return -1;
  }
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
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
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
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
  auto entitlements = EntitlementDatabase();
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
  auto mySqlConnection = std::make_unique<MySql::Connection>(
    mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
    mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema);
  auto administrationServer = AdministrationServletContainer(
    Initialize(serviceLocatorClient.Get(),
    Initialize(serviceLocatorClient.Get(), organizationName, entitlements,
    Initialize(Initialize(std::move(mySqlConnection), accountSource)))),
    Initialize(administrationServerConnectionInitializer.m_interface,
    Ref(socketThreadPool)),
    std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
    Ref(timerThreadPool)));
  try {
    administrationServer.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening server: " << e.what() << endl;
    return -1;
  }
  try {
    auto administrationService = JsonObject();
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
