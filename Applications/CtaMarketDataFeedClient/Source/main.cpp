#include <unordered_map>
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
#include <boost/throw_exception.hpp>
#include "CtaMarketDataFeedClient/CtaConfiguration.hpp"
#include "CtaMarketDataFeedClient/CtaMarketDataFeedClient.hpp"
#include "CtaMarketDataFeedClient/CtaProtocolClient.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
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

namespace {
  using ApplicationFeedChannel = WrapperChannel<MulticastSocketChannel*,
    QueuedReader<SharedBuffer, MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient = CtaProtocolClient<ApplicationFeedChannel*>;
  using ApplicationCtaMarketDataFeedClient = CtaMarketDataFeedClient<
    ApplicationMarketDataFeedClient::Client*, ApplicationProtocolClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv,
      "1.0-r" CTA_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto definitionsClient = ApplicationDefinitionsClient(
      serviceLocatorClient.Get());
    auto samplingTime = Extract<time_duration>(config, "sampling");
    auto marketDataFeedClient = ApplicationMarketDataFeedClient(
      serviceLocatorClient.Get(), samplingTime, DefaultCountries::US());
    auto host = Extract<IpAddress>(config, "host");
    auto interface = Extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receiveBufferSize = Extract<int>(config, "receive_buffer",
      DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_maxDatagramSize = Extract<int>(config, "mtu",
      options.m_maxDatagramSize);
    auto multicastSocketChannel = TryOrNest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join CTA multicast group."));
    auto feedChannel = ApplicationFeedChannel(&multicastSocketChannel,
      &multicastSocketChannel.GetReader());
    auto protocolClient = ApplicationProtocolClient(&feedChannel);
    auto countryDatabase = definitionsClient->LoadCountryDatabase();
    auto marketDatabase = definitionsClient->LoadMarketDatabase();
    auto ctaConfig = CtaConfiguration::Parse(config, countryDatabase,
      marketDatabase);
    auto feedClient = ApplicationCtaMarketDataFeedClient(ctaConfig,
      marketDataFeedClient.Get(), &protocolClient);
    WaitForKillEvent();
    serviceLocatorClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
