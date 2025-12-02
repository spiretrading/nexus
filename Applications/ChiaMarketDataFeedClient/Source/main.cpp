#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel = WrapperChannel<
    MulticastSocketChannel*, QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationProtocolClient =
    PitchProtocolClient<ApplicationFeedChannel*>;
  using ApplicationChiaMarketDataFeedClient = ChiaMarketDataFeedClient<
    ApplicationMarketDataFeedClient*, ApplicationProtocolClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  ChiaConfiguration parse_configuration(const YAML::Node& config) {
    return try_or_nest([&] {
      auto chia_config = ChiaConfiguration();
      chia_config.m_is_logging_messages =
        extract<bool>(config, "enable_logging", false);
      auto primary_venue_entry =
        DEFAULT_VENUES.from_display_name(extract<std::string>(config, "venue"));
      chia_config.m_country = primary_venue_entry.m_country_code;
      chia_config.m_primary_venue = primary_venue_entry.m_venue;
      auto disseminating_venue_entry = DEFAULT_VENUES.from_display_name(
        extract<std::string>(config, "disseminating_market"));
      chia_config.m_disseminating_venue = disseminating_venue_entry.m_venue;
      chia_config.m_mpid = extract<std::string>(
        config, "mpid", disseminating_venue_entry.m_display_name);
      chia_config.m_is_time_and_sale_feed =
        extract<bool>(config, "is_time_and_sale", false);
      return chia_config;
    }, std::runtime_error("Unable to parse CHIA configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" CHIA_MARKET_DATA_FEED_CLIENT_VERSION
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
    }, std::runtime_error("Unable to join CHIA multicast group."));
    auto [
      retransmission_host, retransmission_username, retransmission_password] =
        [&] {
          if(config["retransmission_host"]) {
            return std::tuple(
              make_optional(extract<IpAddress>(config, "retransmission_host")),
              extract<std::string>(config, "retransmission_username"),
              extract<std::string>(config, "retransmission_password"));
          }
          return std::tuple(
            optional<IpAddress>(), std::string(), std::string());
        }();
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, &multicast_socket_channel.get_reader());
    auto protocol_client = ApplicationProtocolClient(&feed_channel);
    auto feed_configuration = parse_configuration(config);
    auto feed_client = ApplicationChiaMarketDataFeedClient(
      feed_configuration, &market_data_feed_client, &protocol_client);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
