#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/ChiaMarketDataFeedClient.hpp"
#include "ChiaMarketDataFeedClient/PitchProtocolClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::MarketDataService;

namespace {
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient =
    PitchProtocolClient<ApplicationFeedChannel*>;
  using ApplicationChiaMarketDataFeedClient = ChiaMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*, ApplicationProtocolClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  ChiaConfiguration ParseConfiguration(
      const YAML::Node& config, const MarketDatabase& marketDatabase) {
    return TryOrNest([&] {
      auto chiaConfig = ChiaConfiguration();
      chiaConfig.m_isLoggingMessages =
        Extract<bool>(config, "enable_logging", false);
      auto primaryMarketEntry =
        marketDatabase.FromDisplayName(Extract<std::string>(config, "market"));
      chiaConfig.m_country = primaryMarketEntry.m_countryCode;
      chiaConfig.m_primaryMarket = primaryMarketEntry.m_code;
      auto disseminatingMarketEntry = marketDatabase.FromDisplayName(
        Extract<std::string>(config, "disseminating_market"));
      chiaConfig.m_disseminatingMarket = disseminatingMarketEntry.m_code;
      chiaConfig.m_mpid = Extract<std::string>(
        config, "mpid", disseminatingMarketEntry.m_displayName);
      chiaConfig.m_isTimeAndSaleFeed =
        Extract<bool>(config, "is_time_and_sale", false);
      return chiaConfig;
    }, std::runtime_error("Unable to parse CHIA configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" CHIA_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto definitionsClient =
      ApplicationDefinitionsClient(serviceLocatorClient.Get());
    auto samplingTime = Extract<time_duration>(config, "sampling");
    auto marketDataFeedClient = ApplicationMarketDataFeedClient(
      serviceLocatorClient.Get(), samplingTime, DefaultCountries::AU());
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize =
      Extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize =
      Extract<int>(config, "mtu", options.m_maxDatagramSize);
    auto multicastSocketChannel = TryOrNest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join CHIA multicast group."));
    auto [retransmissionHost, retransmissionUsername, retransmissionPassword] =
      [&] {
        if(config["retransmission_host"]) {
          return std::tuple(
            make_optional(Extract<IpAddress>(config, "retransmission_host")),
            Extract<std::string>(config, "retransmission_username"),
            Extract<std::string>(config, "retransmission_password"));
        }
        return std::tuple(optional<IpAddress>(), std::string(), std::string());
      }();
    auto feedChannel = ApplicationFeedChannel(
      &multicastSocketChannel, &multicastSocketChannel.GetReader());
    auto protocolClient = ApplicationProtocolClient(&feedChannel);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase);
    auto feedClient = ApplicationChiaMarketDataFeedClient(
      feedConfiguration, marketDataFeedClient.Get(), &protocolClient);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
