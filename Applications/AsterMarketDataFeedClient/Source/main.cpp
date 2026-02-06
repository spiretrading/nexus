#include <iostream>
#include <Beam/Queues/Queue.hpp>
#include <Beam/WebServices/TcpSocketChannelFactory.hpp>
#include <Beam/WebServices/Uri.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include "AsterMarketDataFeedClient/AsterMarketDataFeedClient.hpp"
#include "AsterMarketDataFeedClient/AsterWebClient.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationAsterMarketDataFeedClient = AsterMarketDataFeedClient<
    AsterWebClient<std::unique_ptr<Channel>>*,
    ApplicationMarketDataFeedClient*>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(
      argc, argv, "1.0-r" ASTER_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto sampling_time = extract<time_duration>(config, "sampling");
    auto http_base_uri = extract<Uri>(config, "http_base_uri");
    auto ws_base_uri = extract<Uri>(config, "ws_base_uri");
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), sampling_time, DefaultCountries::US);
    auto aster_client = AsterWebClient(
      TcpSocketChannelFactory(), http_base_uri, ws_base_uri);
    auto feed_client = ApplicationAsterMarketDataFeedClient(
      &aster_client, &market_data_feed_client);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
