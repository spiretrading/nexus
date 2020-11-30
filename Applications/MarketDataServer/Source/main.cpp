#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/Streamable.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/AsyncHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataFeedServlet.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServlet.hpp"
#include "Nexus/MarketDataService/SessionCachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
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
  using SqlDataStore = SqlHistoricalDataStore<SqlConnection<MySql::Connection>>;
  using RegistryServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<MetaMarketDataRegistryServlet<
      MarketDataRegistry*, SessionCachedHistoricalDataStore<
        AsyncHistoricalDataStore<SqlDataStore*>*>,
      ApplicationAdministrationClient::Client*>,
      ApplicationServiceLocatorClient::Client*, NativePointerPolicy>,
    TcpServerSocket, BinarySender<SharedBuffer>, NullEncoder,
    std::shared_ptr<LiveTimer>>;
  using BaseRegistryServlet = MarketDataRegistryServlet<
    RegistryServletContainer, MarketDataRegistry*,
    SessionCachedHistoricalDataStore<AsyncHistoricalDataStore<SqlDataStore*>*>,
    ApplicationAdministrationClient::Client*>;
  using FeedServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
      MetaMarketDataFeedServlet<BaseRegistryServlet*>,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
    std::shared_ptr<LiveTimer>>;

  JsonValue ParseCountries(const YAML::Node& config,
      const CountryDatabase& countryDatabase) {
    return TryOrNest([&] {
      auto countries = std::vector<JsonValue>();
      for(auto item : config) {
        auto code = item.as<std::string>();
        auto country = ParseCountryCode(code, countryDatabase);
        if(country == CountryCode::NONE) {
          throw std::runtime_error("Country code not found: " + code);
        }
        countries.push_back(static_cast<double>(
          static_cast<std::uint16_t>(country)));
      }
      if(countries.empty()) {
        return JsonValue(JsonNull());
      }
      return JsonValue(countries);
    }, std::runtime_error("Error parsing countries."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "0.9-r" MARKET_DATA_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto administrationClient = ApplicationAdministrationClient(
      serviceLocatorClient.Get());
    auto countryDatabase = definitionsClient->LoadCountryDatabase();
    auto registryServiceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "registry_server"),
        MarketDataService::REGISTRY_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'registry_server'."));
    auto feedServiceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "feed_server"),
        MarketDataService::FEED_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'feed_server'."));
    auto countries = TryOrNest([&] {
      if(auto countriesNode = config["countries"]) {
        return ParseCountries(countriesNode,
          definitionsClient->LoadCountryDatabase());
      }
      return JsonValue(JsonNull());
    }, std::runtime_error("Error parsing section 'countries'"));
    if(countries != JsonNull()) {
      registryServiceConfig.m_properties["countries"] = countries;
      feedServiceConfig.m_properties["countries"] = countries;
    }
    auto mySqlConfig = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto historicalDataStore = SqlDataStore([=] {
      return SqlConnection(MySql::Connection(mySqlConfig.m_address.GetHost(),
        mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
        mySqlConfig.m_password, mySqlConfig.m_schema));
    });
    auto asyncDataStore = AsyncHistoricalDataStore<SqlDataStore*>(
      &historicalDataStore);
    auto cacheBlockSize = Extract<int>(config, "cache_block_size", 1000);
    auto marketDataRegistry = MarketDataRegistry();
    auto baseRegistryServlet = BaseRegistryServlet(&*administrationClient,
      &marketDataRegistry, Initialize(&asyncDataStore, cacheBlockSize));
    auto registryServer = RegistryServletContainer(Initialize(
      serviceLocatorClient.Get(), &baseRegistryServlet),
      Initialize(registryServiceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, registryServiceConfig);
    auto feedServer = FeedServletContainer(Initialize(
      serviceLocatorClient.Get(), &baseRegistryServlet),
      Initialize(feedServiceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, feedServiceConfig);
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
