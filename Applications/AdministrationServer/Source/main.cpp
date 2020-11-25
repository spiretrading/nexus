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
using namespace Viper;

namespace {
  using AdministrationServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaAdministrationServlet<
    ApplicationServiceLocatorClient::Client*, CachedAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<MySql::Connection>>>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

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
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" ADMINISTRATION_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto mySqlConfig = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        AdministrationService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = TryOrNest([&] {
      return ApplicationDefinitionsClient(Ref(*serviceLocatorClient));
    }, std::runtime_error("Unable to connect to the definitions service."));
    auto entitlements = TryOrNest([&] {
      return ParseEntitlements(GetNode(config, "entitlements"),
        definitionsClient->LoadCurrencyDatabase(), serviceLocatorClient);
    }, std::runtime_error("Error parsing entitlements."));
    auto accountSource =
      [&] (unsigned int id) {
        return serviceLocatorClient->LoadDirectoryEntry(id);
      };
    auto server = TryOrNest([&] {
      auto mySqlConnection = MakeSqlConnection(MySql::Connection(
        mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
        mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema));
      auto administrationServer = optional<AdministrationServletContainer>();
      return AdministrationServletContainer(Initialize(serviceLocatorClient.Get(),
        Initialize(serviceLocatorClient.Get(), entitlements,
        Initialize(Initialize(std::move(mySqlConnection), accountSource)))),
        Initialize(serviceConfig.m_interface),
        std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    }, std::runtime_error("Error opening server."));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
