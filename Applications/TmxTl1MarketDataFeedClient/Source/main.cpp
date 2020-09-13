#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Parsers/Parse.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace TCLAP;

namespace {
  using BaseMarketDataFeedClient = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationRetransmissionServerChannel = UdpSocketChannel;
  using ApplicationTmxTl1ServiceAccessClient =
    TmxTl1ServiceAccessClient<ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = TmxTl1MarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationTmxTl1ServiceAccessClient*>;
  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  std::unordered_map<std::string, std::string> LoadMpidMappings(
      const YAML::Node& config) {
    auto mappings = std::unordered_map<std::string, std::string>();
    for(auto& node : config) {
      auto source = Extract<std::string>(node, "source");
      auto name = Extract<std::string>(node, "name");
      mappings.insert(std::pair(source, name));
    }
    return mappings;
  }

  TmxTl1Configuration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, ptime currentDate,
      const local_time::tz_database& timeZones) {
    auto configTimezone = Extract<std::string>(config, "time_zone",
      "Eastern_Time");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
    }
    auto tmxTl1Config = TmxTl1Configuration();
    tmxTl1Config.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    auto& market = marketDatabase.FromDisplayName(
      Extract<std::string>(config, "market"));
    tmxTl1Config.m_market = market.m_code;
    tmxTl1Config.m_country = market.m_countryCode;
    tmxTl1Config.m_timeOffset = -GetUtcOffset(currentDate, *timeZone);
    return tmxTl1Config;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION
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
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto timeClient = std::unique_ptr<LiveNtpTimeClient>();
  try {
    auto timeServices = serviceLocatorClient->Locate(TimeService::SERVICE_NAME);
    if(timeServices.empty()) {
      std::cerr << "No time services available." << std::endl;
      return -1;
    }
    auto& timeService = timeServices.front();
    auto ntpPool = Parse<std::vector<IpAddress>>(get<std::string>(
      timeService.GetProperties().At("addresses")));
    timeClient = MakeLiveNtpTimeClient(ntpPool);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  auto baseMarketDataFeedClient = optional<BaseMarketDataFeedClient>();
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::CA(),
      *serviceLocatorClient);
    if(!marketDataService.is_initialized()) {
      std::cerr << "No market data services available." << std::endl;
      return -1;
    }
    auto marketDataAddresses = Parse<std::vector<IpAddress>>(
      get<std::string>(marketDataService->GetProperties().At("addresses")));
    auto samplingTime = Extract<time_duration>(config, "sampling");
    baseMarketDataFeedClient.emplace(Initialize(marketDataAddresses),
      SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
        Ref(*serviceLocatorClient)), Initialize(samplingTime),
      Initialize(seconds(10)));
  } catch(const std::exception& e) {
    std::cerr << "Error initializing client: " << e.what() << std::endl;
    return -1;
  }
  auto multicastSocketChannel = optional<MulticastSocketChannel>();
  try {
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    multicastSocketChannel.emplace(host, interface, options);
  } catch(const std::exception& e) {
    std::cerr << "Error initializing multicast socket: " << e.what() <<
      std::endl;
    return -1;
  }
  auto tmxTl1Config = TmxTl1Configuration();
  auto feedChannel = optional<ApplicationFeedChannel>();
  auto serviceAccessClient = optional<ApplicationTmxTl1ServiceAccessClient>();
  auto marketDataFeedClient = optional<ApplicationMarketDataFeedClient>();
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    tmxTl1Config = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    feedChannel.emplace(multicastSocketChannel.get_ptr(),
      &multicastSocketChannel->GetReader());
    serviceAccessClient.emplace(feedChannel.get_ptr());
    marketDataFeedClient.emplace(tmxTl1Config,
      baseMarketDataFeedClient.get_ptr(), serviceAccessClient.get_ptr());
  } catch(const std::exception& e) {
    std::cerr << "Error initializing TMX TL1 configuration: " << e.what() <<
      std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
