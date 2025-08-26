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
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include "Nexus/ChartingService/ChartingServlet.hpp"
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ChartingServletContainer =
    ServiceProtocolServletContainer<MetaAuthenticationServletAdapter<
      MetaChartingServlet<ApplicationMarketDataClient::Client*>,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
      BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>,
      std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "1.0-r" CHARTING_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto service_config = TryOrNest([&] {
      return ServiceConfiguration::Parse(
        GetNode(config, "server"), CHARTING_SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto service_locator_client =
      MakeApplicationServiceLocatorClient(GetNode(config, "service_locator"));
    auto market_data_client =
      ApplicationMarketDataClient(service_locator_client.Get());
    auto charting_server = ChartingServletContainer(Initialize(
      service_locator_client.Get(), Initialize(market_data_client.Get())),
      Initialize(service_config.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*service_locator_client, service_config);
    WaitForKillEvent();
    service_locator_client->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
