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
using namespace std;
using namespace TCLAP;

namespace {
  using BaseMarketDataFeedClient = MarketDataFeedClient<string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient = ChiaMmdProtocolClient<
    ApplicationFeedChannel*, TcpSocketChannel>;
  using ApplicationMarketDataFeedClient = ChiaMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationProtocolClient*>;

  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  ChiaConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, const ptime& currentTime,
      const local_time::tz_database& timeZones) {
    ChiaConfiguration chiaConfig;
    chiaConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    auto primaryMarketEntry = marketDatabase.FromDisplayName(
      Extract<string>(config, "market"));
    chiaConfig.m_country = primaryMarketEntry.m_countryCode;
    chiaConfig.m_primaryMarket = primaryMarketEntry.m_code;
    auto disseminatingMarketEntry = marketDatabase.FromDisplayName(
      Extract<string>(config, "disseminating_market"));
    chiaConfig.m_disseminatingMarket = disseminatingMarketEntry.m_code;
    chiaConfig.m_mpid = Extract<string>(config, "mpid",
      disseminatingMarketEntry.m_displayName);
    auto configTimezone = Extract<string>(config, "time_zone",
      "Australian_Eastern_Standard_Time");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Time zone not found."});
    }
    ptime serverDate{
      AdjustDateTime(currentTime, "UTC", configTimezone, timeZones).date(),
      seconds(0)};
    chiaConfig.m_timeOrigin = AdjustDateTime(serverDate, configTimezone,
      "UTC", timeZones);
    chiaConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
      false);
    return chiaConfig;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" CHIA_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
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
  ApplicationDefinitionsClient definitionsClient;
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  unique_ptr<LiveNtpTimeClient> timeClient;
  try {
    auto timeServices = serviceLocatorClient->Locate(TimeService::SERVICE_NAME);
    if(timeServices.empty()) {
      cerr << "No time services available." << endl;
      return -1;
    }
    auto& timeService = timeServices.front();
    auto ntpPool = FromString<vector<IpAddress>>(get<string>(
      timeService.GetProperties().At("addresses")));
    timeClient = MakeLiveNtpTimeClient(ntpPool, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const  std::exception& e) {
    cerr << "Unable to initialize NTP client: " << e.what() << endl;
    return -1;
  }
  try {
    timeClient->Open();
  } catch(const std::exception&) {
    cerr << "NTP service unavailable." << endl;
    return -1;
  }
  boost::optional<BaseMarketDataFeedClient> baseMarketDataFeedClient;
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::AU(),
      *serviceLocatorClient);
    if(!marketDataService.is_initialized()) {
      cerr << "No market data services available." << endl;
      return -1;
    }
    auto marketDataAddresses = FromString<vector<IpAddress>>(
      get<string>(marketDataService->GetProperties().At("addresses")));
    auto samplingTime = Extract<time_duration>(config, "sampling");
    baseMarketDataFeedClient.emplace(
      Initialize(marketDataAddresses, Ref(socketThreadPool)),
      SessionAuthenticator<ApplicationServiceLocatorClient::Client>{
        Ref(*serviceLocatorClient)},
      Initialize(samplingTime, Ref(timerThreadPool)),
      Initialize(seconds{10}, Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Unable to initialize market data client: " << e.what() << endl;
    return -1;
  }
  boost::optional<MulticastSocketChannel> multicastSocketChannel;
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
    cerr << "Unable to initialize multicast socket: " << e.what() << endl;
    return -1;
  }
  boost::optional<IpAddress> retransmissionHost;
  string retransmissionUsername;
  string retransmissionPassword;
  try {
    if(config["retransmission_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "retransmission_host");
      retransmissionUsername = Extract<string>(config,
        "retransmission_username");
      retransmissionPassword = Extract<string>(config,
        "retransmission_password");
    }
  } catch(const std::exception& e) {
    cerr << "Unable to initialize retransmission: " << e.what() << endl;
    return -1;
  }
  ApplicationFeedChannel feedChannel{multicastSocketChannel.get_ptr(),
    &multicastSocketChannel->GetReader()};
  ApplicationProtocolClient protocolClient{&feedChannel, retransmissionUsername,
    retransmissionPassword,
    [&] () -> std::unique_ptr<TcpSocketChannel> {
      if(retransmissionHost.is_initialized()) {
        return std::make_unique<TcpSocketChannel>(*retransmissionHost,
          Ref(socketThreadPool));
      }
      return nullptr;
    }
  };
  boost::optional<ApplicationMarketDataFeedClient> feedClient;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    feedClient.emplace(feedConfiguration, baseMarketDataFeedClient.get_ptr(),
      &protocolClient);
  } catch(const std::exception& e) {
    cerr << "Unable to initialize market data feed client: " << e.what() <<
      endl;
    return -1;
  }
  try {
    feedClient->Open();
  } catch(const std::exception& e) {
    cerr << "Error opening client: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
