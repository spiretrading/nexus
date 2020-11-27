#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "HkexMarketDataFeedClient/HkexConfiguration.hpp"
#include "HkexMarketDataFeedClient/HkexMarketDataFeedClient.hpp"
#include "HkexMarketDataFeedClient/HkexProtocolClient.hpp"
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
  using ApplicationProtocolClient = HkexProtocolClient<ApplicationFeedChannel*>;
  using ApplicationHkexMarketDataFeedClient = HkexMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*, ApplicationProtocolClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  HkexConfiguration ParseConfiguration(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    return TryOrNest([&] {
      auto hkexConfig = HkexConfiguration();
      hkexConfig.m_enableLogging = false;
      hkexConfig.m_market = marketDatabase.FromCode(DefaultMarkets::HKEX());
      hkexConfig.m_mpid = "HKEX";
      return hkexConfig;
    }, std::runtime_error("Failed to parse HKEX configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" HKEX_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      Ref(*serviceLocatorClient));
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto samplingTime = Extract<time_duration>(config, "sampling");
    auto marketDataFeedClient = ApplicationMarketDataFeedClient(
      Ref(*serviceLocatorClient), samplingTime, DefaultCountries::HK());
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    auto multicastSocketChannel = TryOrNest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join HKEX multicast group."));
    auto feedChannel = ApplicationFeedChannel(&multicastSocketChannel,
      &multicastSocketChannel.GetReader());
    auto protocolClient = ApplicationProtocolClient(&feedChannel);
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto feedConfiguration = ParseConfiguration(config, marketDatabase);
    auto feedClient = ApplicationHkexMarketDataFeedClient(
      std::move(feedConfiguration), marketDataFeedClient.Get(),
      &protocolClient);
    WaitForKillEvent();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
