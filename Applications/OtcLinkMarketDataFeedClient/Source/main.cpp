#include <Beam/IO/QueuedReader.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Network/MulticastSocketChannel.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  using ApplicationFeedChannel = WrapperChannel<
    MulticastSocketChannel*, QueuedReader<MulticastSocketChannel::Reader*>>;
  using ApplicationOtcLinkClient = OtcLinkClient<ApplicationFeedChannel*>;
  static const auto DEFAULT_RECEIVE_BUFFER_SIZE = std::size_t(16777216);
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv,
      "1.0-r" OTC_LINK_MARKET_DATA_FEED_CLIENT_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto host = extract<IpAddress>(config, "host");
    auto interface = extract<IpAddress>(config, "interface");
    auto options = MulticastSocketOptions();
    options.m_receive_buffer_size =
      extract<int>(config, "receive_buffer", DEFAULT_RECEIVE_BUFFER_SIZE);
    options.m_max_datagram_size =
      extract<int>(config, "mtu", options.m_max_datagram_size);
    auto multicast_socket_channel = try_or_nest([&] {
      return MulticastSocketChannel(host, interface, options);
    }, std::runtime_error("Unable to join NEOE multicast group."));
    auto feed_channel = ApplicationFeedChannel(
      &multicast_socket_channel, &multicast_socket_channel.get_reader());
    auto otc_link_client = ApplicationOtcLinkClient(&feed_channel);
    while(!received_kill_event()) {
      auto sequence = std::uint32_t();
      auto message = otc_link_client.read(out(sequence));
      std::cout << sequence << ": " << static_cast<int>(message.m_type) <<
        " - " << message.m_size << " bytes" << std::endl;
    }
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
