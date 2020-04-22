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
using namespace std;
using namespace TCLAP;

namespace {
  using ApplicationSoupBinTcpClient =
    SoupBinTcpClient<TcpSocketChannel, LiveTimer>;
  using BaseMarketDataFeedClient = MarketDataFeedClient<string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationMoldUdp64Client = MoldUdp64Client<ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = AsxItchMarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationMoldUdp64Client*,
    ApplicationSoupBinTcpClient*>;

  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  AsxItchConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    AsxItchConfiguration asxConfig;
    asxConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    asxConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
      false);
    asxConfig.m_market = marketDatabase.FromDisplayName(
      Extract<string>(config, "market"));
    asxConfig.m_defaultMpid = Extract<string>(config, "mpid", "");
    asxConfig.m_consolidateMpids = Extract<bool>(config, "consolidate_mpids",
      false);
    return asxConfig;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
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
  boost::optional<BaseMarketDataFeedClient> baseMarketDataFeedClient;
  try {
    auto marketDataService = FindMarketDataFeedService(DefaultCountries::AU(),
      *serviceLocatorClient);
    if(!marketDataService.is_initialized()) {
      cerr << "No market data services available." << endl;
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
  ApplicationFeedChannel feedChannel{multicastSocketChannel.get_ptr(),
    &multicastSocketChannel->GetReader()};
  ApplicationMoldUdp64Client moldClient{&feedChannel};
  boost::optional<ApplicationSoupBinTcpClient> glimpseClient;
  try {
    auto glimpseHost = Extract<IpAddress>(config, "glimpse_host");
    auto glimpseTimeout = Extract<time_duration>(config, "glimpse_timeout",
      seconds{10});
    glimpseClient.emplace(Initialize(glimpseHost, Ref(socketThreadPool)),
      Initialize(glimpseTimeout, Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    cerr << "Unable to initialize Glimpse client: " << e.what() << endl;
    return -1;
  }
  boost::optional<ApplicationMarketDataFeedClient> feedClient;
  CurrencyDatabase currencyDatabase;
  try {
    currencyDatabase = definitionsClient->LoadCurrencyDatabase();
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase);
    feedConfiguration.m_glimpseUsername = Extract<string>(config, "username");
    feedConfiguration.m_glimpsePassword = Extract<string>(config, "password");
    feedClient.emplace(feedConfiguration, Ref(currencyDatabase),
      baseMarketDataFeedClient.get_ptr(), &moldClient, glimpseClient.get_ptr());
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
