#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkMarketDataFeedClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel = WrapperChannel<
    MulticastSocketChannel*, QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationRecoveryFeedChannel =
    WrapperChannel<std::unique_ptr<MulticastSocketChannel>,
      QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationOtcLinkClient = OtcLinkClient<ApplicationFeedChannel*>;
  using ApplicationRecoveryOtcLinkClient =
    OtcLinkClient<std::unique_ptr<ApplicationRecoveryFeedChannel>>;
  using ApplicationOtcLinkRecoveryClient =
    OtcLinkRecoveryClient<std::unique_ptr<TcpSocketChannel>>;
  using ApplicationOtcLinkMarketDataFeedClient = OtcLinkMarketDataFeedClient<
    ApplicationMarketDataFeedClient*, ApplicationOtcLinkClient*,
    ApplicationOtcLinkRecoveryClient*,
    std::unique_ptr<ApplicationRecoveryOtcLinkClient>>;
  static const auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);

  OtcLinkConfiguration parse_configuration(const YAML::Node& config) {
    return try_or_nest([&] {
      auto otc_link_config = OtcLinkConfiguration();
      otc_link_config.m_is_logging_messages =
        extract<bool>(config, "enable_logging", false);
      otc_link_config.m_venue = DEFAULT_VENUES.from_display_name(
        extract<std::string>(config, "venue")).m_venue;
      return otc_link_config;
    }, std::runtime_error("Unable to parse OTC Link configuration."));
  }

  MulticastSocketChannel make_multicast_channel(const YAML::Node& config) {
    auto host = extract<IpAddress>(config, "host");
    auto interface = extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    return try_or_nest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join OTC Link multicast group."));
  }

  ApplicationOtcLinkRecoveryClient make_recovery_client(
      const YAML::Node& config) {
    auto sender_comp_id = extract<std::string>(config, "sender_comp_id");
    auto recovery_address = extract<IpAddress>(config, "recovery_address");
    return ApplicationOtcLinkRecoveryClient(sender_comp_id, [=] {
      return std::make_unique<TcpSocketChannel>(recovery_address);
    });
  }

  auto make_snapshot_client_builder(const YAML::Node& config) {
    auto host = extract<IpAddress>(config, "recovery_host");
    auto interface = extract<IpAddress>(config, "recovery_interface");
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size = extract<int>(
      config, "recovery_receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size = extract<int>(config, "recovery_mtu");
    return [=] {
      auto channel = try_or_nest([&] {
        return std::make_unique<MulticastSocketChannel>(
          host, interface, options);
      }, std::runtime_error(
        "Unable to join recovery OTC Link multicast group."));
      auto reader = &channel->get_reader();
      auto feed_channel = std::make_unique<ApplicationRecoveryFeedChannel>(
        std::move(channel), reader);
      return std::make_unique<ApplicationRecoveryOtcLinkClient>(
        std::move(feed_channel));
    };
  }
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" OTC_LINK_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto sampling_time = extract<time_duration>(config, "sampling");
    auto market_data_feed_client = ApplicationMarketDataFeedClient(
      Ref(service_locator_client), sampling_time, DefaultCountries::US);
    auto multicast_socket_channel = make_multicast_channel(config);
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, &multicast_socket_channel.get_reader());
    auto otc_link_client = ApplicationOtcLinkClient(&feed_channel);
    auto recovery_client = make_recovery_client(config);
    auto snapshot_client_builder = make_snapshot_client_builder(config);
    auto feed_configuration = parse_configuration(config);
    auto feed_client = ApplicationOtcLinkMarketDataFeedClient(
      feed_configuration, &market_data_feed_client, &otc_link_client,
      &recovery_client, snapshot_client_builder);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
