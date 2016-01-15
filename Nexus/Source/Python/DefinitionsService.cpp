#include "Nexus/Python/DefinitionsService.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::Python;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = DefinitionsClient<SessionBuilder>;

  VirtualDefinitionsClient* BuildClient(
      VirtualServiceLocatorClient* serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(*serviceLocatorClient,
      DefinitionsService::SERVICE_NAME);
    auto delay = false;
    SessionBuilder sessionBuilder(Ref(*serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          LiveTimer delayTimer(seconds(3), Ref(*GetTimerThreadPool()));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<TcpSocketChannel>(addresses,
          Ref(*GetSocketThreadPool()));
      },
      [=] {
        return std::make_unique<LiveTimer>(seconds(10),
          Ref(*GetTimerThreadPool()));
      });
    auto baseClient = std::make_unique<Client>(sessionBuilder);
    return new WrapperDefinitionsClient<std::unique_ptr<Client>>{
      std::move(baseClient)};
  }
}

void Nexus::Python::ExportDefinitionsClient() {
  class_<VirtualDefinitionsClient, boost::noncopyable>("DefinitionsClient",
      no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("load_minimum_spire_client_version", BlockingFunction(
      &VirtualDefinitionsClient::LoadMinimumSpireClientVersion))
    .def("load_country_database",
      BlockingFunction(&VirtualDefinitionsClient::LoadCountryDatabase))
    .def("load_time_zone_database",
      BlockingFunction(&VirtualDefinitionsClient::LoadTimeZoneDatabase))
    .def("load_currency_database",
      BlockingFunction(&VirtualDefinitionsClient::LoadCurrencyDatabase))
    .def("load_destination_database",
      BlockingFunction(&VirtualDefinitionsClient::LoadDestinationDatabase))
    .def("load_market_database",
      BlockingFunction(&VirtualDefinitionsClient::LoadMarketDatabase))
    .def("load_exchange_rates",
      BlockingFunction(&VirtualDefinitionsClient::LoadExchangeRates))
    .def("load_compliance_rule_schemas",
      BlockingFunction(&VirtualDefinitionsClient::LoadComplianceRuleSchemas))
    .def("open", BlockingFunction(&VirtualDefinitionsClient::Open))
    .def("close", BlockingFunction(&VirtualDefinitionsClient::Close));
}

void Nexus::Python::ExportDefinitionsService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".definitions_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("definitions_service") = nestedModule;
  scope parent = nestedModule;
  ExportDefinitionsClient();
}
