#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpServerSocket.hpp>
#include <Beam/Network/UdpSocketChannel.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationServletAdapter.hpp>
#include <Beam/Services/ServiceProtocolServletContainer.hpp>
#include <Beam/Sql/MySqlConfig.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include <Beam/Utilities/ApplicationInterrupt.hpp>
#include <Beam/Utilities/Expect.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/TelemetryService/TelemetryServlet.hpp"
#include "Nexus/TelemetryService/SqlTelemetryDataStore.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::TelemetryService;
using namespace Viper;

namespace {
  using ApplicationDataStore =
    SqlTelemetryDataStore<SqlConnection<MySql::Connection>>;
  using TelemetryServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
      MetaTelemetryServlet<std::unique_ptr<LiveNtpTimeClient>,
        ApplicationAdministrationClient::Client*, ApplicationDataStore*>,
      ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;
}

int main(int argc, const char** argv) {
  try {
    auto config = ParseCommandLine(argc, argv, "0.9-r" TELEMETRY_SERVER_VERSION
      "\nCopyright (C) 2021 Spire Trading Inc.");
    auto serviceConfig = TryOrNest([&] {
      return ServiceConfiguration::Parse(GetNode(config, "server"),
        RiskService::SERVICE_NAME);
    }, std::runtime_error("Error parsing section 'server'."));
    auto serviceLocatorClient = MakeApplicationServiceLocatorClient(
      GetNode(config, "service_locator"));
    auto administrationClient = ApplicationAdministrationClient(
      serviceLocatorClient.Get());
    auto timeClient = MakeLiveNtpTimeClientFromServiceLocator(
      *serviceLocatorClient);
    auto mySqlConfig = TryOrNest([&] {
      return MySqlConfig::Parse(GetNode(config, "data_store"));
    }, std::runtime_error("Error parsing section 'data_store'."));
    auto dataStore = ApplicationDataStore([=] {
      return SqlConnection(MySql::Connection(mySqlConfig.m_address.GetHost(),
        mySqlConfig.m_address.GetPort(), mySqlConfig.m_username,
        mySqlConfig.m_password, mySqlConfig.m_schema));
    });
    auto server = TelemetryServletContainer(Initialize(
      serviceLocatorClient.Get(), Initialize(std::move(timeClient),
      administrationClient.Get(), &dataStore)),
      Initialize(serviceConfig.m_interface),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10)));
    Register(*serviceLocatorClient, serviceConfig);
    WaitForKillEvent();
    serviceLocatorClient->Close();
    administrationClient->Close();
  } catch(...) {
    ReportCurrentException();
    return -1;
  }
  return 0;
}
