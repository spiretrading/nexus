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
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/ChiaMarketDataFeedClient.hpp"
#include "ChiaMarketDataFeedClient/ChiaMdProtocolClient.hpp"
#include "ChiaMarketDataFeedClient/ChiaMmdProtocolClient.hpp"
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
  using ApplicationProtocolClient = ChiaMmdProtocolClient<
    ApplicationFeedChannel*, TcpSocketChannel>;
  using ApplicationMarketDataFeedClient = ChiaMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationProtocolClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  ChiaConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, ptime currentTime,
      const local_time::tz_database& timeZones) {
    auto chiaConfig = ChiaConfiguration();
    chiaConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    auto primaryMarketEntry = marketDatabase.FromDisplayName(
      Extract<std::string>(config, "market"));
    chiaConfig.m_country = primaryMarketEntry.m_countryCode;
    chiaConfig.m_primaryMarket = primaryMarketEntry.m_code;
    auto disseminatingMarketEntry = marketDatabase.FromDisplayName(
      Extract<std::string>(config, "disseminating_market"));
    chiaConfig.m_disseminatingMarket = disseminatingMarketEntry.m_code;
    chiaConfig.m_mpid = Extract<std::string>(config, "mpid",
      disseminatingMarketEntry.m_displayName);
    auto configTimezone = Extract<std::string>(config, "time_zone",
      "Australian_Eastern_Standard_Time");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
    }
    auto serverDate = ptime(
      AdjustDateTime(currentTime, "UTC", configTimezone, timeZones).date(),
      seconds(0));
    chiaConfig.m_timeOrigin = AdjustDateTime(serverDate, configTimezone,
      "UTC", timeZones);
    chiaConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
      false);
    return chiaConfig;
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" CHIA_MARKET_DATA_FEED_CLIENT_VERSION
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
  } catch(const  std::exception& e) {
    std::cerr << "Unable to initialize NTP client: " << e.what() << std::endl;
    return -1;
  }
  auto baseMarketDataFeedClient = optional<BaseMarketDataFeedClient>();
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::AU(),
      *serviceLocatorClient);
    if(!marketDataService) {
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
      Initialize(seconds{10}, Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data client: " << e.what() <<
      std::endl;
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
    multicastSocketChannel.emplace(host, interface, options,
      Ref(socketThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize multicast socket: " << e.what() <<
      std::endl;
    return -1;
  }
  auto retransmissionHost = optional<IpAddress>();
  auto retransmissionUsername = std::string();
  auto retransmissionPassword = std::string();
  try {
    if(config["retransmission_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "retransmission_host");
      retransmissionUsername = Extract<std::string>(config,
        "retransmission_username");
      retransmissionPassword = Extract<std::string>(config,
        "retransmission_password");
    }
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize retransmission: " << e.what() <<
      std::endl;
    return -1;
  }
  auto feedChannel = optional<ApplicationFeedChannel>();
  auto protocolClient = optional<ApplicationProtocolClient>();
  auto feedClient = optional<ApplicationMarketDataFeedClient>();
  try {
    feedChannel.emplace(multicastSocketChannel.get_ptr(),
      &multicastSocketChannel->GetReader());
    protocolClient.emplace(feedChannel.get_ptr(), retransmissionUsername,
      retransmissionPassword,
      [&] () -> std::unique_ptr<TcpSocketChannel> {
        if(retransmissionHost) {
          return std::make_unique<TcpSocketChannel>(*retransmissionHost,
            Ref(socketThreadPool));
        }
        return nullptr;
      });
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    feedClient.emplace(feedConfiguration, baseMarketDataFeedClient.get_ptr(),
      protocolClient.get_ptr());
  } catch(const std::exception& e) {
    std::cerr << "Unable to initialize market data feed client: " << e.what() <<
      std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
