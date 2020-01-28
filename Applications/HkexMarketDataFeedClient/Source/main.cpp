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
#include "HkexMarketDataFeedClient/HkexConfiguration.hpp"
#include "HkexMarketDataFeedClient/HkexMarketDataFeedClient.hpp"
#include "HkexMarketDataFeedClient/HkexProtocolClient.hpp"
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
  using ApplicationProtocolClient = HkexProtocolClient<ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = HkexMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationProtocolClient*>;

  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  HkexConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, const ptime& currentTime,
      const local_time::tz_database& timeZones) {
    auto hkexConfig = HkexConfiguration();
    hkexConfig.m_enableLogging = false;
    hkexConfig.m_market = GetDefaultMarketDatabase().FromCode(
      DefaultMarkets::HKEX());
    hkexConfig.m_disseminatingMarket = DefaultMarkets::HKEX();
    hkexConfig.m_mpid = "HKEX";
    return hkexConfig;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" HKEX_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2011 Eidolon Systems Inc."};
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
  auto definitionsClient = ApplicationDefinitionsClient();
  try {
    definitionsClient.BuildSession(Ref(*serviceLocatorClient),
      Ref(socketThreadPool), Ref(timerThreadPool));
    definitionsClient->Open();
  } catch(const std::exception&) {
    cerr << "Unable to connect to the definitions service." << endl;
    return -1;
  }
  auto timeClient = unique_ptr<LiveNtpTimeClient>();
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
  auto baseMarketDataFeedClient = std::optional<BaseMarketDataFeedClient>();
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::HK(),
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
      SessionAuthenticator<ApplicationServiceLocatorClient::Client>(
        Ref(*serviceLocatorClient)),
      Initialize(samplingTime, Ref(timerThreadPool)),
      Initialize(seconds(10), Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Unable to initialize market data client: " << e.what() << endl;
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
    cerr << "Unable to initialize multicast socket: " << e.what() << endl;
    return -1;
  }
  auto retransmissionHost = std::optional<IpAddress>();
  try {
    if(config["retransmission_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "retransmission_host");
    }
  } catch(const std::exception& e) {
    cerr << "Unable to initialize retransmission: " << e.what() << endl;
    return -1;
  }
  auto snapshotHost = std::optional<IpAddress>();
  try {
    if(config["snapshot_host"]) {
      retransmissionHost = Extract<IpAddress>(config, "snapshot_host");
    }
  } catch(const std::exception& e) {
    cerr << "Unable to initialize snapshot: " << e.what() << endl;
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
