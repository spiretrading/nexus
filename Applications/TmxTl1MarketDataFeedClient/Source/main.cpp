#include <unordered_map>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"
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
  using ApplicationTmxTl1ServiceAccessClient =
    TmxTl1ServiceAccessClient<ApplicationFeedChannel*>;
  using ApplicationTmxTl1MarketDataFeedClient = TmxTl1MarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*,
    ApplicationTmxTl1ServiceAccessClient*>;

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
    }, std::runtime_error("Failed to parse MPID mappings."));
  }

  TmxTl1Configuration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, ptime currentDate,
      const local_time::tz_database& timeZones) {
    return TryOrNest([&] {
      auto configTimezone = Extract<std::string>(config, "time_zone",
        "Eastern_Time");
      auto timeZone = timeZones.time_zone_from_region(configTimezone);
      if(timeZone == nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
      }
      auto tmxTl1Config = TmxTl1Configuration();
      tmxTl1Config.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
        false);
      auto& market = marketDatabase.FromDisplayName(
        Extract<std::string>(config, "market"));
      tmxTl1Config.m_market = market.m_code;
      tmxTl1Config.m_country = market.m_countryCode;
      tmxTl1Config.m_timeOffset = -GetUtcOffset(currentDate, *timeZone);
      return tmxTl1Config;
    }, std::runtime_error("Failed to parse TMX TL1 configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION
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
    }, std::runtime_error("Unable to join TMX TL1 multicast group."));
    auto feedChannel = ApplicationFeedChannel(&multicastSocketChannel,
      &multicastSocketChannel.GetReader());
    auto serviceAccessClient = ApplicationTmxTl1ServiceAccessClient(
      &feedChannel);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto tmxTl1Config = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    auto feedClient = ApplicationTmxTl1MarketDataFeedClient(tmxTl1Config,
      marketDataFeedClient.Get(), &serviceAccessClient);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
