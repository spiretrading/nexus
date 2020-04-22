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
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <tclap/CmdLine.h>
#include "JpxFlexMarketDataFeedClient/JpxFlexConfiguration.hpp"
#include "JpxFlexMarketDataFeedClient/JpxFlexMarketDataFeedClient.hpp"
#include "JpxFlexMarketDataFeedClient/JpxFlexProtocolClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
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
  using ApplicationProtocolClient = JpxFlexProtocolClient<
    ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = JpxFlexMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationProtocolClient*>;

  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  JpxFlexConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, const ptime& currentTime,
      const local_time::tz_database& timeZones) {
    auto jpxConfig = JpxFlexConfiguration();
    jpxConfig.m_enableLogging = Extract<bool>(config, "enable_logging",
      false);
    jpxConfig.m_market = ParseMarketEntry(
      Extract<std::string>(config, "market"), marketDatabase);
    jpxConfig.m_disseminatingMarket = ParseMarketCode(
      Extract<std::string>(config, "disseminating_market"), marketDatabase);
    jpxConfig.m_mpid = Extract<std::string>(config, "mpid");
    auto configTimezone = Extract<std::string>(config, "time_zone");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
    }
    jpxConfig.m_utcOffset = currentTime - AdjustDateTime(currentTime, "UTC",
      configTimezone, timeZones);
    return jpxConfig;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" JPX_FLEX_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Eidolon Systems Inc.");
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
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
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
    timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  try {
    timeClient->Open();
  } catch(const std::exception&) {
    std::cerr << "NTP service unavailable." << std::endl;
    return -1;
  }
  auto baseMarketDataFeedClient = std::optional<BaseMarketDataFeedClient>();
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::HK(),
      *serviceLocatorClient);
    if(!marketDataService.is_initialized()) {
      std::cerr << "No market data services available." << std::endl;
      return -1;
    }
    auto marketDataAddresses = Parse<std::vector<IpAddress>>(
      get<std::string>(marketDataService->GetProperties().At("addresses")));
    auto samplingTime = Extract<time_duration>(config, "sampling");
    baseMarketDataFeedClient.emplace(
      Initialize(marketDataAddresses, Ref(socketThreadPool)),
      SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
        Ref(*serviceLocatorClient)),
      Initialize(samplingTime, Ref(timerThreadPool)),
      Initialize(seconds(10), Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data client: " << e.what() <<
      std::endl;
    return -1;
  }
  auto multicastSocketChannel = std::optional<MulticastSocketChannel>();
  try {
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    multicastSocketChannel.emplace(host, interface, Ref(socketThreadPool));
    auto receiverSettings =
      multicastSocketChannel->GetSocket().GetReceiverSettings();
    receiverSettings.m_receiveBufferSize = Extract<int>(config,
      "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    receiverSettings.m_maxDatagramSize = Extract<int>(config, "mtu",
      UdpSocketReceiver::Settings::DEFAULT_DATAGRAM_SIZE);
    multicastSocketChannel->GetSocket().SetReceiverSettings(receiverSettings);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize multicast socket: " << e.what() <<
      std::endl;
    return -1;
  }
  auto retransmissionHost = std::optional<IpAddress>();
  try {
    if(config["retransmission_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "retransmission_host");
    }
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize retransmission: " << e.what() <<
      std::endl;
    return -1;
  }
  auto snapshotHost = std::optional<IpAddress>();
  try {
    if(config["snapshot_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "snapshot_host");
    }
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize snapshot: " << e.what() << std::endl;
    return -1;
  }
  auto feedChannel = ApplicationFeedChannel(&*multicastSocketChannel,
    &multicastSocketChannel->GetReader());
  auto protocolClient = ApplicationProtocolClient(&feedChannel);
  auto feedClient = std::optional<ApplicationMarketDataFeedClient>();
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    feedClient.emplace(std::move(feedConfiguration), &*baseMarketDataFeedClient,
      &protocolClient);
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data feed client: " << e.what() <<
      std::endl;
    return -1;
  }
  try {
    feedClient->Open();
  } catch(const std::exception& e) {
    std::cerr << "Error opening client: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
