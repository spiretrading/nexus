#include <unordered_map>
#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/throw_exception.hpp>
#include "CseMarketDataFeedClient/CseMarketDataFeedClient.hpp"
#include "CseMarketDataFeedClient/CseServiceAccessClient.hpp"
#include "Nexus/Definitions/DefaultTimeZoneDatabase.hpp"
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
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
  using ApplicationCseServiceAccessClient = CseServiceAccessClient<
    ApplicationFeedChannel*, TcpSocketChannel,
    ApplicationRetransmissionServerChannel>;
  using ApplicationCseMarketDataFeedClient = CseMarketDataFeedClient<
    ApplicationMarketDataFeedClient*, ApplicationCseServiceAccessClient*,
    LiveNtpTimeClient*>;

  static constexpr auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  std::vector<SecurityInfo> parse_security_info_list(const std::string& path) {
    return try_or_nest([&] {
      auto config = load_file(path);
      auto securities = std::vector<SecurityInfo>();
      for(auto node : config) {
        auto symbol = extract<std::string>(node, "symbol");
        auto name = extract<std::string>(node, "name");
        auto board_lot = extract<Quantity>(node, "board_lot");
        auto info = SecurityInfo();
        info.m_name = name;
        info.m_security = Security(symbol, DefaultVenues::CSE);
        info.m_board_lot = board_lot;
        securities.push_back(std::move(info));
      }
      return securities;
    }, std::runtime_error("Unable to parse security info list."));
  }

  std::unordered_map<std::string, std::string> load_mpid_mappings(
      const YAML::Node& config) {
    return try_or_nest([&] {
      auto mappings = std::unordered_map<std::string, std::string>();
      for(auto node : config) {
        auto source = extract<std::string>(node, "source");
        auto name = extract<std::string>(node, "name");
        mappings.insert(std::pair(source, name));
      }
      return mappings;
    }, std::runtime_error("Unable to parse MPID mappings."));
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

  CseConfiguration parse_configuration(const YAML::Node& config) {
    return try_or_nest([&] {
      auto time_zone =
        extract<std::string>(config, "time_zone", "America/Toronto");
      auto cse_config = CseConfiguration();
      cse_config.m_is_logging_messages =
        extract<bool>(config, "enable_logging", false);
      cse_config.m_is_time_and_sale_feed =
        extract<bool>(config, "is_time_and_sale", false);
      cse_config.m_time_offset = -get_utc_offset(
        get_default_time_zone_database(), time_zone);
      cse_config.m_venue = parse_venue(extract<std::string>(config, "venue"));
      if(auto mpid_mappings = config["mpid_mappings"]) {
        cse_config.m_mpid_mappings = load_mpid_mappings(mpid_mappings);
      }
      return cse_config;
    }, std::runtime_error("Unable to parse CSE configuration."));
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" CSE_MARKET_DATA_FEED_CLIENT_VERSION
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
    }, std::runtime_error("Unable to join CSE multicast group."));
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, &multicast_socket_channel.get_reader());
    auto retransmission_client_address =
      extract<IpAddress>(config, "retransmission_request_address");
    auto retransmission_server_address =
      extract<IpAddress>(config, "retransmission_response_address");
    auto retransmission_client_channel_builder =
      [=] (Out<std::optional<TcpSocketChannel>> channel) {
        channel->emplace(retransmission_client_address);
      };
    auto service_access_config = CseServiceAccessConfiguration();
    service_access_config.m_enable_retransmission =
      extract<bool>(config, "enable_retransmission", false);
    service_access_config.m_max_retransmission_count =
      extract<int>(config, "max_retransmissions", 10);
    service_access_config.m_max_retransmission_block =
      extract<int>(config, "retransmission_block_size", 20000);
    auto cse_config = parse_configuration(config);
    auto symbol_list = extract<std::string>(config, "symbol_list");
    auto securities = parse_security_info_list(symbol_list);
    for(auto& security : securities) {
      cse_config.m_securities.insert(security.m_security.get_symbol());
      market_data_feed_client.add(security);
    }
    auto service_access_client = ApplicationCseServiceAccessClient(
      service_access_config, &feed_channel,
      retransmission_client_channel_builder, init(retransmission_server_address,
        IpAddress("0.0.0.0", retransmission_server_address.get_port())));
    auto feed_client = ApplicationCseMarketDataFeedClient(
      cse_config, &market_data_feed_client, &service_access_client,
      time_client.get());
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
