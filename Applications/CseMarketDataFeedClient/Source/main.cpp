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
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "CseMarketDataFeedClient/CseMarketDataFeedClient.hpp"
#include "CseMarketDataFeedClient/CseServiceAccessClient.hpp"
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
  using ApplicationRetransmissionServerChannel = UdpSocketChannel;
  using ApplicationCseServiceAccessClient = CseServiceAccessClient<
    ApplicationFeedChannel*, TcpSocketChannel,
    ApplicationRetransmissionServerChannel>;
  using ApplicationMarketDataFeedClient = CseMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationCseServiceAccessClient*,
    LiveNtpTimeClient*>;
  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  std::vector<SecurityInfo> ParseSecurityInfoList(const std::string& path) {
    auto config = Require(LoadFile, path);
    auto securities = std::vector<SecurityInfo>();
    for(auto node : config) {
      auto symbol = Extract<string>(node, "symbol");
      auto name = Extract<string>(node, "name");
      auto boardLot = Extract<Quantity>(node, "board_lot");
      auto info = SecurityInfo();
      info.m_name = name;
      info.m_security = Security(symbol, DefaultMarkets::CSE(),
        DefaultCountries::CA());
      info.m_boardLot = boardLot;
      securities.push_back(std::move(info));
    }
    return securities;
  }

  std::unordered_map<string, string> LoadMpidMappings(
      const YAML::Node& config) {
    auto mappings = std::unordered_map<string, string>();
    for(auto node : config) {
      auto source = Extract<string>(node, "source");
      auto name = Extract<string>(node, "name");
      mappings.insert(make_pair(source, name));
    }
    return mappings;
  }

  CseConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, const ptime& currentDate,
      const local_time::tz_database& timeZones) {
    auto configTimezone = Extract<string>(config, "time_zone", "Eastern_Time");
    auto timeZone = timeZones.time_zone_from_region(configTimezone);
    if(timeZone == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{"Time zone not found."});
    }
    auto cseConfig = CseConfiguration();
    cseConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    cseConfig.m_timeOffset = -GetUtcOffset(currentDate, *timeZone);
    cseConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
      false);
    if(auto mpidMappings = config["mpid_mappings"]) {
      cseConfig.m_mpidMappings = LoadMpidMappings(mpidMappings);
    }
    return cseConfig;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" CSE_MARKET_DATA_FEED_CLIENT_VERSION
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
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::CA(),
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
    cerr << "Error initializing client: " << e.what() << endl;
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
    cerr << "Error initializing multicast socket: " << e.what() << endl;
    return -1;
  }
  ApplicationFeedChannel feedChannel{multicastSocketChannel.get_ptr(),
    &multicastSocketChannel->GetReader()};
  IpAddress retransmissionClientAddress;
  IpAddress retransmissionServerAddress;
  try {
    retransmissionClientAddress = Extract<IpAddress>(config,
      "retransmission_request_address");
    retransmissionServerAddress = Extract<IpAddress>(config,
      "retransmission_response_address");
  } catch(const std::exception& e) {
    cerr << "Error initializing retransmission: " << e.what() << endl;
    return -1;
  }
  auto retransmissionClientChannelBuilder =
    [&socketThreadPool, retransmissionClientAddress] (
        Out<std::optional<TcpSocketChannel>> channel) {
      channel->emplace(retransmissionClientAddress, Ref(socketThreadPool));
    };
  CseServiceAccessConfiguration serviceAccessConfig;
  serviceAccessConfig.m_enableRetransmission =
    Extract<bool>(config, "enable_retransmission", false);
  serviceAccessConfig.m_maxRetransmissionCount =
    Extract<int>(config, "max_retransmissions", 10);
  serviceAccessConfig.m_maxRetransmissionBlock =
    Extract<int>(config, "retransmission_block_size", 20000);
  ApplicationCseServiceAccessClient serviceAccessClient{serviceAccessConfig,
    &feedChannel, retransmissionClientChannelBuilder,
    Initialize(retransmissionServerAddress,
    IpAddress{"0.0.0.0", retransmissionServerAddress.GetPort()},
    Ref(socketThreadPool))};
  CseConfiguration cseConfig;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    cseConfig = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
  } catch(const std::exception& e) {
    cerr << "Error initializing CSE configuration: " << e.what() << endl;
    return -1;
  }
  std::vector<SecurityInfo> securities;
  try {
    auto symbolList = Extract<string>(config, "symbol_list");
    securities = ParseSecurityInfoList(symbolList);
  } catch(const std::exception& e) {
    cerr << "Error loading security info list: " << e.what() << endl;
    return -1;
  }
  for(auto& security : securities) {
    cseConfig.m_securities.insert(security.m_security.GetSymbol());
  }
  ApplicationMarketDataFeedClient marketDataFeedClient{cseConfig,
    baseMarketDataFeedClient.get_ptr(), &serviceAccessClient, timeClient.get()};
  try {
    marketDataFeedClient.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening client: " << e.what() << endl;
    return -1;
  }
  for(auto& security : securities) {
    baseMarketDataFeedClient->Add(security);
  }
  WaitForKillEvent();
  return 0;
}
