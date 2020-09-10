#include <fstream>
#include <iostream>
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
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "AsxItchMarketDataFeedClient/AsxItchMarketDataFeedClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::MoldUdp64;
using namespace Nexus::SoupBinTcp;
using namespace TCLAP;

namespace {
  using ApplicationSoupBinTcpClient =
    SoupBinTcpClient<TcpSocketChannel, LiveTimer>;
  using BaseMarketDataFeedClient = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationMoldUdp64Client = MoldUdp64Client<ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = AsxItchMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationMoldUdp64Client*,
    ApplicationSoupBinTcpClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  AsxItchConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    auto asxConfig = AsxItchConfiguration();
    asxConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    asxConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
      false);
    asxConfig.m_market = marketDatabase.FromDisplayName(Extract<std::string>(
      config, "market"));
    asxConfig.m_defaultMpid = Extract<std::string>(config, "mpid", "");
    asxConfig.m_consolidateMpids = Extract<bool>(config, "consolidate_mpids",
      false);
    return asxConfig;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
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
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(GetNode(
      config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
  } catch(const std::exception&) {
    std::cerr << "Unable to connect to the definitions service." << std::endl;
    return -1;
  }
  auto baseMarketDataFeedClient = optional<BaseMarketDataFeedClient>();
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::AU(),
      *serviceLocatorClient);
    if(!marketDataService.is_initialized()) {
      std::cerr << "No market data services available." << std::endl;
      return -1;
    }
    auto marketDataAddresses = Parse<std::vector<IpAddress>>(get<std::string>(
      marketDataService->GetProperties().At("addresses")));
    auto samplingTime = Extract<time_duration>(config, "sampling");
    baseMarketDataFeedClient.emplace(
      Initialize(marketDataAddresses, Ref(socketThreadPool)),
      SessionAuthenticator<ApplicationServiceLocatorClient::Client>{
        Ref(*serviceLocatorClient)},
      Initialize(samplingTime, Ref(timerThreadPool)),
      Initialize(seconds(10), Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data client: " << e.what() <<
      std::endl;
    return -1;
  }
  auto multicastSocketChannel = optional<MulticastSocketChannel>();
  auto feedChannel = optional<ApplicationFeedChannel>();
  auto moldClient = optional<ApplicationMoldUdp64Client>();
  try {
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    multicastSocketChannel.emplace(host, interface, options,
      Ref(socketThreadPool));
    feedChannel.emplace(multicastSocketChannel.get_ptr(),
      &multicastSocketChannel->GetReader());
    moldClient.emplace(feedChannel.get_ptr());
  } catch(const std::exception& e) {
    std::cerr << "Unable to join multicast group: " << e.what() << std::endl;
    return -1;
  }
  auto glimpseClient = optional<ApplicationSoupBinTcpClient>();
  auto feedClient = optional<ApplicationMarketDataFeedClient>();
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase);
    auto glimpseHost = Extract<IpAddress>(config, "glimpse_host");
    auto glimpseTimeout = Extract<time_duration>(config, "glimpse_timeout",
      seconds(10));
    feedConfiguration.m_glimpseUsername = Extract<std::string>(config,
      "username");
    feedConfiguration.m_glimpsePassword = Extract<std::string>(config,
      "password");
    try {
      glimpseClient.emplace(feedConfiguration.m_glimpseUsername,
        feedConfiguration.m_glimpsePassword,
        Initialize(glimpseHost, Ref(socketThreadPool)),
        Initialize(glimpseTimeout, Ref(timerThreadPool)));
    } catch(const std::exception& e) {
      std::cerr << "Unable to open the Glimpse client: " << e.what() <<
        std::endl;
      return -1;
    }
    auto currencyDatabase = definitionsClient->LoadCurrencyDatabase();
    feedClient.emplace(feedConfiguration, Ref(currencyDatabase),
      baseMarketDataFeedClient.get_ptr(), moldClient.get_ptr(),
      glimpseClient.get_ptr());
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data feed client: " << e.what() <<
      std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
