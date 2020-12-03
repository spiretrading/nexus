#include <unordered_map>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMarketDataFeedClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpServiceAccessClient.hpp"
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
  using ApplicationRetransmissionServerChannel = UdpSocketChannel;
  using ApplicationTmxIpServiceAccessClient = TmxIpServiceAccessClient<
    ApplicationFeedChannel*, TcpSocketChannel,
    ApplicationRetransmissionServerChannel>;
  using ApplicationTmxIpMarketDataFeedClient = TmxIpMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*,
    ApplicationTmxIpServiceAccessClient*, LiveNtpTimeClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  std::unordered_map<std::string, std::string> LoadMpidMappings(
      const YAML::Node& config) {
    return TryOrNest([&] {
      auto mappings = std::unordered_map<std::string, std::string>();
      for(auto& node : config) {
        auto source = Extract<std::string>(node, "source");
        auto name = Extract<std::string>(node, "name");
        mappings.insert(std::pair(source, name));
      }
      return mappings;
    }, std::runtime_error("Error parsing MPID mappings."));
  }

  TmxIpConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, ptime currentDate,
      const local_time::tz_database& timeZones) {
    return TryOrNest([&] {
      auto configTimezone = Extract<std::string>(config, "time_zone",
        "Eastern_Time");
      auto timeZone = timeZones.time_zone_from_region(configTimezone);
      if(timeZone == nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
      }
      auto tmxIpConfig = TmxIpConfiguration();
      tmxIpConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
        false);
      tmxIpConfig.m_timeOffset = -GetUtcOffset(currentDate, *timeZone);
      tmxIpConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
        false);
      auto& market = marketDatabase.FromDisplayName(
        Extract<std::string>(config, "market"));
      tmxIpConfig.m_market = market.m_code;
      tmxIpConfig.m_country = market.m_countryCode;
      tmxIpConfig.m_useBrokerNumberAsKey = Extract<bool>(config,
        "use_broker_number", false);
      tmxIpConfig.m_defaultMpid = Extract<std::string>(config, "mpid", "");
      tmxIpConfig.m_consolidateMpids = Extract<bool>(config, "consolidate_mpids",
        false);
      if(auto mpidMappings = config["mpid_mappings"]) {
        tmxIpConfig.m_mpidMappings = LoadMpidMappings(mpidMappings);
      }
      tmxIpConfig.m_isNeoBook = market.m_code == DefaultMarkets::NEOE();
      return tmxIpConfig;
    }, std::runtime_error("Failed to parse TMX IP configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" TMX_IP_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto samplingTime = Extract<time_duration>(config, "sampling");
    auto marketDataFeedClient = ApplicationMarketDataFeedClient(
      serviceLocatorClient.Get(), samplingTime, DefaultCountries::CA());
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    auto multicastSocketChannel = TryOrNest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join TMX IP multicast group."));
    auto feedChannel = ApplicationFeedChannel(&multicastSocketChannel,
      &multicastSocketChannel.GetReader());
    auto retransmissionClientAddress = Extract<IpAddress>(config,
      "retransmission_request_address");
    auto retransmissionServerAddress = Extract<IpAddress>(config,
      "retransmission_response_address");
    auto retransmissionClientChannelBuilder =
      [=] (Out<std::optional<TcpSocketChannel>> channel) {
        channel->emplace(retransmissionClientAddress);
      };
    auto serviceAccessConfig = TmxIpServiceAccessConfiguration();
    serviceAccessConfig.m_enableRetransmission = Extract<bool>(
      config, "enable_retransmission", false);
    serviceAccessConfig.m_maxRetransmissionCount = Extract<int>(
      config, "max_retransmissions", 10);
    serviceAccessConfig.m_maxRetransmissionBlock = Extract<int>(
      config, "retransmission_block_size", 20000);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto tmxIpConfig = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    auto serviceAccessClient = ApplicationTmxIpServiceAccessClient(
      serviceAccessConfig, &feedChannel, retransmissionClientChannelBuilder,
      Initialize(retransmissionServerAddress,
      IpAddress("0.0.0.0", retransmissionServerAddress.GetPort())));
    auto feedClient = ApplicationTmxIpMarketDataFeedClient(tmxIpConfig,
      marketDataFeedClient.Get(), &serviceAccessClient, timeClient.get());
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
