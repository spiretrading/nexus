#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/TimeService/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ChartingServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
      MetaChartingServlet<ApplicationMarketDataClient*>,
      ApplicationServiceLocatorClient*>, TcpServerSocket,
      BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
      std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = parse_command_line(argc, argv, "1.0-r" CHARTING_SERVER_VERSION
      "\nCopyright (C) 2026 Spire Trading Inc.");
    auto service_config = try_or_nest([&] {
      return ServiceConfiguration::parse(
        get_node(config, "server"), CHARTING_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client = ApplicationServiceLocatorClient(
      ServiceLocatorClientConfig::parse(get_node(config, "service_locator")));
    auto market_data_client =
      ApplicationMarketDataClient(Ref(service_locator_client));
    auto charting_server = ChartingServletContainer(
      init(&service_locator_client, init(&market_data_client)),
      init(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    add(service_locator_client, service_config);
    wait_for_kill_event();
    service_locator_client.close();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
