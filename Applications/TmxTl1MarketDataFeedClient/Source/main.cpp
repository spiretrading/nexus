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
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"
#include "TmxTl1MarketDataFeedClient/Version.hpp"

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
  using ApplicationTmxTl1ServiceAccessClient =
    TmxTl1ServiceAccessClient<ApplicationFeedChannel*>;
  using ApplicationMarketDataFeedClient = TmxTl1MarketDataFeedClient<
    BaseMarketDataFeedClient*, ApplicationTmxTl1ServiceAccessClient*>;
  static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 16777216;

  std::unordered_map<string, string> LoadMpidMappings(
      const YAML::Node& config) {
    std::unordered_map<string, string> mappings;
    for(auto& node : config) {
      auto source = Extract<string>(node, "source");
      auto name = Extract<string>(node, "name");
      mappings.insert(make_pair(source, name));
    }
    return mappings;
  }

  TmxTl1Configuration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    TmxTl1Configuration tmxTl1Config;
    tmxTl1Config.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
      false);
    auto& market = marketDatabase.FromDisplayName(
      Extract<string>(config, "market"));
    tmxTl1Config.m_market = market.m_code;
    tmxTl1Config.m_country = market.m_countryCode;
    return tmxTl1Config;
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd{"", ' ', "1.0-r" TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION
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
  YAML::Node config;
  try {
    ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
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
  optional<BaseMarketDataFeedClient> baseMarketDataFeedClient;
  try {
    auto marketDataServices = serviceLocatorClient->Locate(
      MarketDataService::FEED_SERVICE_NAME);
    if(marketDataServices.empty()) {
      cerr << "No market data services available." << endl;
      return -1;
    }
    auto& marketDataService = marketDataServices.front();
    auto marketDataAddresses = FromString<vector<IpAddress>>(
      get<string>(marketDataService.GetProperties().At("addresses")));
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
  optional<MulticastSocketChannel> multicastSocketChannel;
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
  ApplicationTmxTl1ServiceAccessClient serviceAccessClient{&feedChannel};
  TmxTl1Configuration tmxTl1Config;
  try {
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    tmxTl1Config = ParseConfiguration(config, marketDatabase);
  } catch(const std::exception& e) {
    cerr << "Error initializing TMX TL1 configuration: " << e.what() << endl;
    return -1;
  }
  ApplicationMarketDataFeedClient marketDataFeedClient{tmxTl1Config,
    baseMarketDataFeedClient.get_ptr(), &serviceAccessClient};
  try {
    marketDataFeedClient.Open();
  } catch(const std::exception& e) {
    cerr << "Error opening client: " << e.what() << endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
