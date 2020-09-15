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
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
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
using namespace TCLAP;
using namespace Viper;

namespace {
  using AdministrationServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaAdministrationServlet<
    ApplicationServiceLocatorClient::Client*, CachedAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<MySql::Connection>>>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct AdministrationServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void AdministrationServerConnectionInitializer::Initialize(
      const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      AdministrationService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }

  EntitlementDatabase ParseEntitlements(const YAML::Node& config,
      const CurrencyDatabase& currencies,
      ApplicationServiceLocatorClient& serviceLocatorClient) {
    auto entitlementsDirectory = LoadOrCreateDirectory(*serviceLocatorClient,
      "entitlements", DirectoryEntry::GetStarDirectory());
    auto database = EntitlementDatabase();
    for(auto entitlementConfig : config) {
      auto entry = EntitlementDatabase::Entry();
      entry.m_name = Extract<std::string>(entitlementConfig, "name");
      auto price = Money::FromValue(Extract<std::string>(entitlementConfig,
        "price"));
      if(!price.is_initialized()) {
        throw std::runtime_error("Invalid entitlement price.");
      }
      entry.m_price = *price;
      entry.m_currency = currencies.FromCode(
        Extract<std::string>(entitlementConfig, "currency")).m_id;
      auto groupName = Extract<std::string>(entitlementConfig, "group");
      entry.m_groupEntry = LoadOrCreateDirectory(*serviceLocatorClient,
        groupName, entitlementsDirectory);
      auto applicability = GetNode(entitlementConfig, "applicability");
      for(auto applicabilityConfig : applicability) {
        auto market = MarketCode(
          Extract<std::string>(applicabilityConfig, "market", ""));
        auto source = MarketCode(
          Extract<std::string>(applicabilityConfig, "source"));
        auto key = EntitlementKey(market, source);
        auto messages = GetNode(applicabilityConfig, "messages");
        for(auto messageConfig : messages) {
          auto message = messageConfig.as<std::string>();
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
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto administrationServerConnectionInitializer =
    AdministrationServerConnectionInitializer();
  try {
    administrationServerConnectionInitializer.Initialize(
      GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address);
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto entitlements = EntitlementDatabase();
  try {
    entitlements = ParseEntitlements(GetNode(config, "entitlements"),
      definitionsClient->LoadCurrencyDatabase(), serviceLocatorClient);
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'entitlements': " << e.what() <<
      std::endl;
    return -1;
  }
  auto accountSource =
    [&] (unsigned int id) {
      return serviceLocatorClient->LoadDirectoryEntry(id);
    };
  auto mySqlConnection = MakeSqlConnection(MySql::Connection(
    mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
    mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema));
  auto administrationServer = optional<AdministrationServletContainer>();
  try {
    administrationServer.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(serviceLocatorClient.Get(), entitlements,
      Initialize(Initialize(std::move(mySqlConnection), accountSource)))),
      Initialize(administrationServerConnectionInitializer.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto service = JsonObject();
    service["addresses"] = lexical_cast<std::string>(
      Stream(administrationServerConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(
      administrationServerConnectionInitializer.m_serviceName, service);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
