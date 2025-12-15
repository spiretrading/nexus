#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationSoupBinTcpClient =
    SoupBinTcpClient<TcpSocketChannel, LiveTimer>;
  using ApplicationFeedChannel = WrapperChannel<
    MulticastSocketChannel*, QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationMoldUdp64Client = MoldUdp64Client<ApplicationFeedChannel*>;
  using ApplicationAsxItchMarketDataFeedClient = AsxItchMarketDataFeedClient<
    ApplicationMarketDataFeedClient*, ApplicationMoldUdp64Client*,
    ApplicationSoupBinTcpClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  AsxItchConfiguration parse_configuration(const YAML::Node& config) {
    return try_or_nest([&] {
      auto asx_config = AsxItchConfiguration();
      asx_config.m_is_logging_messages =
        extract<bool>(config, "enable_logging", false);
      asx_config.m_is_time_and_sale_feed =
        extract<bool>(config, "is_time_and_sale", false);
      asx_config.m_venue = DEFAULT_VENUES.from_display_name(
        extract<std::string>(config, "venue"));
      asx_config.m_default_mpid = extract<std::string>(config, "mpid", "");
      asx_config.m_consolidate_mpids =
        extract<bool>(config, "consolidate_mpids", false);
      return asx_config;
    }, std::runtime_error("Failed to parse ASX ITCH configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" ASX_ITCH_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto sampling_time = extract<time_duration>(config, "sampling");
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), sampling_time, DefaultCountries::AU);
    auto host = extract<IpAddress>(config, "host");
    auto interface = extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    auto multicast_socket_channel = try_or_nest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join ASX ITCH multicast group."));
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, init(&multicast_socket_channel.get_reader()));
    auto mold_client = ApplicationMoldUdp64Client(&feed_channel);
    auto feed_configuration = parse_configuration(config);
    auto glimpse_host = extract<IpAddress>(config, "glimpse_host");
    auto glimpse_timeout =
      extract<time_duration>(config, "glimpse_timeout", seconds(10));
    feed_configuration.m_glimpse_username =
      extract<std::string>(config, "username");
    feed_configuration.m_glimpse_password =
      extract<std::string>(config, "password");
    auto glimpse_client = try_or_nest([&] {
      return ApplicationSoupBinTcpClient(feed_configuration.m_glimpse_username,
        feed_configuration.m_glimpse_password, init(glimpse_host),
        init(glimpse_timeout));
    }, std::runtime_error("Unable to connect to GLIMPSE."));
    auto feed_client = ApplicationAsxItchMarketDataFeedClient(
      feed_configuration, &market_data_feed_client, &mold_client,
      &glimpse_client);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
