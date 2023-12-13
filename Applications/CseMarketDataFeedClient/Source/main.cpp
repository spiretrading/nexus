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
#include "CseMarketDataFeedClient/CseMarketDataFeedClient.hpp"
#include "CseMarketDataFeedClient/CseServiceAccessClient.hpp"
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
  using ApplicationRetransmissionServerChannel = UdpSocketChannel;
  using ApplicationCseServiceAccessClient = CseServiceAccessClient<
    ApplicationFeedChannel*, TcpSocketChannel,
    ApplicationRetransmissionServerChannel>;
  using ApplicationCseMarketDataFeedClient = CseMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*,
    ApplicationCseServiceAccessClient*, LiveNtpTimeClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  std::vector<SecurityInfo> ParseSecurityInfoList(const std::string& path) {
    return TryOrNest([&] {
      auto config = Require(LoadFile, path);
      auto securities = std::vector<SecurityInfo>();
      for(auto node : config) {
        auto symbol = Extract<std::string>(node, "symbol");
        auto name = Extract<std::string>(node, "name");
        auto boardLot = Extract<Quantity>(node, "board_lot");
        auto info = SecurityInfo();
        info.m_name = name;
        info.m_security = Security(symbol, DefaultMarkets::CSE(),
          DefaultCountries::CA());
        info.m_boardLot = boardLot;
        securities.push_back(std::move(info));
      }
      return securities;
    }, std::runtime_error("Unable to parse security info list."));
  }

  std::unordered_map<std::string, std::string> LoadMpidMappings(
      const YAML::Node& config) {
    return TryOrNest([&] {
      auto mappings = std::unordered_map<std::string, std::string>();
      for(auto node : config) {
        auto source = Extract<std::string>(node, "source");
        auto name = Extract<std::string>(node, "name");
        mappings.insert(std::pair(source, name));
      }
      return mappings;
    }, std::runtime_error("Unable to parse MPID mappings."));
  }

  CseConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase, ptime currentDate,
      const local_time::tz_database& timeZones) {
    return TryOrNest([&] {
      auto configTimezone = Extract<std::string>(config, "time_zone",
        "Eastern_Time");
      auto timeZone = timeZones.time_zone_from_region(configTimezone);
      if(timeZone == nullptr) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Time zone not found."));
      }
      auto cseConfig = CseConfiguration();
      cseConfig.m_isLoggingMessages = Extract<bool>(config, "enable_logging",
        false);
      cseConfig.m_isTimeAndSaleFeed = Extract<bool>(config, "is_time_and_sale",
        false);
      cseConfig.m_timeOffset = -GetUtcOffset(currentDate, *timeZone);
      cseConfig.m_market =
        ParseMarketCode(Extract<std::string>(config, "market"), marketDatabase);
      if(auto mpidMappings = config["mpid_mappings"]) {
        cseConfig.m_mpidMappings = LoadMpidMappings(mpidMappings);
      }
      return cseConfig;
    }, std::runtime_error("Unable to parse CSE configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" CSE_MARKET_DATA_FEED_CLIENT_VERSION
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
    }, std::runtime_error("Unable to join CSE multicast group."));
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
    auto serviceAccessConfig = CseServiceAccessConfiguration();
    serviceAccessConfig.m_enableRetransmission = Extract<bool>(
      config, "enable_retransmission", false);
    serviceAccessConfig.m_maxRetransmissionCount = Extract<int>(
      config, "max_retransmissions", 10);
    serviceAccessConfig.m_maxRetransmissionBlock = Extract<int>(
      config, "retransmission_block_size", 20000);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto timeZones = definitionsClient->LoadTimeZoneDatabase();
    auto cseConfig = ParseConfiguration(config, marketDatabase,
      timeClient->GetTime(), timeZones);
    auto symbolList = Extract<std::string>(config, "symbol_list");
    auto securities = ParseSecurityInfoList(symbolList);
    for(auto& security : securities) {
      cseConfig.m_securities.insert(security.m_security.GetSymbol());
    }
    auto serviceAccessClient = ApplicationCseServiceAccessClient(
      serviceAccessConfig, &feedChannel, retransmissionClientChannelBuilder,
      Initialize(retransmissionServerAddress,
      IpAddress("0.0.0.0", retransmissionServerAddress.GetPort())));
    auto feedClient = ApplicationCseMarketDataFeedClient(cseConfig,
      marketDataFeedClient.Get(), &serviceAccessClient, timeClient.get());
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
