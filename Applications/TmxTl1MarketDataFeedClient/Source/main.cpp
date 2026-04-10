#include <unordered_map>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1MarketDataFeedClient.hpp"
#include "TmxTl1MarketDataFeedClient/TmxTl1ServiceAccessClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::local_time;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel = WrapperChannel<
    MulticastSocketChannel*, QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationRetransmissionServerChannel = UdpSocketChannel;
  using ApplicationTmxTl1ServiceAccessClient =
    TmxTl1ServiceAccessClient<ApplicationFeedChannel*>;
  using ApplicationTmxTl1MarketDataFeedClient = TmxTl1MarketDataFeedClient<
    ApplicationMarketDataFeedClient*, ApplicationTmxTl1ServiceAccessClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  std::unordered_map<std::string, std::string> load_mpid_mappings(
      const YAML::Node& config) {
    return try_or_nest([&] {
      auto mappings = std::unordered_map<std::string, std::string>();
      for(auto& node : config) {
        auto source = extract<std::string>(node, "source");
        auto name = extract<std::string>(node, "name");
        mappings.insert(std::pair(source, name));
      }
      return mappings;
    }, std::runtime_error("Failed to parse MPID mappings."));
  }

  time_duration get_utc_offset(const tz_database& tz_database,
      const std::string& time_zone) {
    auto tz = tz_database.time_zone_from_region(time_zone);
    if(!tz) {
      throw_with_location(std::runtime_error(
        "Time zone '" + time_zone + "' not found in database."));
    }
    auto current_time = second_clock::universal_time();
    auto local_time = local_date_time(current_time, tz);
    return local_time.local_time() - local_time.utc_time();
  }

  TmxTl1Configuration parse_configuration(const YAML::Node& config) {
    return try_or_nest([&] {
      auto time_zone =
        extract<std::string>(config, "time_zone", "America/Toronto");
      auto tmx_tl1_config = TmxTl1Configuration();
      tmx_tl1_config.m_is_logging_messages =
        extract<bool>(config, "enable_logging", false);
      auto& venue = DEFAULT_VENUES.from(
        parse_venue(extract<std::string>(config, "venue")));
      tmx_tl1_config.m_venue = venue.m_venue;
      tmx_tl1_config.m_country = venue.m_country_code;
      tmx_tl1_config.m_time_offset =
        -get_utc_offset(get_default_time_zone_database(), time_zone);
      return tmx_tl1_config;
    }, std::runtime_error("Failed to parse TMX TL1 configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" TMX_TL1_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto definitions_client =
      ApplicationDefinitionsClient(Ref(service_locator_client));
    auto time_client = make_live_ntp_time_client(service_locator_client);
    auto sampling_time = extract<time_duration>(config, "sampling");
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), sampling_time, DefaultCountries::CA);
    auto host = extract<IpAddress>(config, "host");
    auto interface = extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    auto multicast_socket_channel = try_or_nest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join TMX TL1 multicast group."));
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, &multicast_socket_channel.get_reader());
    auto tmx_tl1_config = parse_configuration(config);
    auto service_access_client =
      ApplicationTmxTl1ServiceAccessClient(&feed_channel);
    auto feed_client = ApplicationTmxTl1MarketDataFeedClient(
      tmx_tl1_config, &market_data_feed_client, &service_access_client);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
