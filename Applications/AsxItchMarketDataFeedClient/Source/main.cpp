#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "AsxItchMarketDataFeedClient/AsxItchMarketDataFeedClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;
using namespace Nexus::MoldUdp64;
using namespace Nexus::SoupBinTcp;

namespace {
  using ApplicationSoupBinTcpClient =
    SoupBinTcpClient<TcpSocketChannel, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationMoldUdp64Client = MoldUdp64Client<ApplicationFeedChannel*>;
  using ApplicationAsxItchMarketDataFeedClient = AsxItchMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*, ApplicationMoldUdp64Client*,
    ApplicationSoupBinTcpClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  AsxItchConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    return TryOrNest([&] {
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
    }, std::runtime_error("Failed to parse ASX ITCH configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto samplingTime = Extract<time_duration>(config, "sampling");
    auto marketDataFeedClient = ApplicationMarketDataFeedClient(
      serviceLocatorClient.Get(), samplingTime, DefaultCountries::AU());
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    auto multicastSocketChannel = TryOrNest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join ASX ITCH multicast group."));
    auto feedChannel = ApplicationFeedChannel(&multicastSocketChannel,
      &multicastSocketChannel.GetReader());
    auto moldClient = ApplicationMoldUdp64Client(&feedChannel);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase);
    auto glimpseHost = Extract<IpAddress>(config, "glimpse_host");
    auto glimpseTimeout = Extract<time_duration>(config, "glimpse_timeout",
      seconds(10));
    feedConfiguration.m_glimpseUsername = Extract<std::string>(config,
      "username");
    feedConfiguration.m_glimpsePassword = Extract<std::string>(config,
      "password");
    auto glimpseClient = TryOrNest([&] {
      return ApplicationSoupBinTcpClient(feedConfiguration.m_glimpseUsername,
        feedConfiguration.m_glimpsePassword, Initialize(glimpseHost),
        Initialize(glimpseTimeout));
    }, std::runtime_error("Unable to connect to GLIMPSE."));
    auto currencyDatabase = definitionsClient->LoadCurrencyDatabase();
    auto feedClient = ApplicationAsxItchMarketDataFeedClient(feedConfiguration,
      Ref(currencyDatabase), marketDataFeedClient.Get(), &moldClient,
      &glimpseClient);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
