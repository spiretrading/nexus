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
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"

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

  struct FromPythonDefinitionsClient : VirtualDefinitionsClient,
      wrapper<VirtualDefinitionsClient> {
    virtual std::string LoadMinimumSpireClientVersion() override final {
      return get_override("load_minimum_spire_client_version")();
    }

    virtual CountryDatabase LoadCountryDatabase() override final {
      return get_override("load_country_database")();
    }

    virtual boost::local_time::tz_database
        LoadTimeZoneDatabase() override final {
      return get_override("load_time_zone_database")();
    }

    virtual CurrencyDatabase LoadCurrencyDatabase() override final {
      return get_override("load_currency_database")();
    }

    virtual DestinationDatabase LoadDestinationDatabase() override final {
      return get_override("load_destination_database")();
    }

    virtual MarketDatabase LoadMarketDatabase() override final {
      return get_override("load_market_database")();
    }

    virtual std::vector<ExchangeRate> LoadExchangeRates() override final {
      return get_override("load_exchange_rates")();
    }

    virtual std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() override final {
      return get_override("load_compliance_rule_schemas")();
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto BuildClient(VirtualServiceLocatorClient& serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      DefinitionsService::SERVICE_NAME);
    auto delay = false;
    SessionBuilder sessionBuilder(Ref(serviceLocatorClient),
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
    return MakeToPythonDefinitionsClient(std::make_unique<Client>(
      sessionBuilder)).release();
  }
}

void Nexus::Python::ExportApplicationDefinitionsClient() {
  class_<ToPythonDefinitionsClient<Client>, bases<VirtualDefinitionsClient>,
    boost::noncopyable>("ApplicationDefinitionsClient", no_init)
    .def("__init__", make_constructor(&BuildClient));
}

void Nexus::Python::ExportDefinitionsClient() {
  class_<FromPythonDefinitionsClient, boost::noncopyable>("DefinitionsClient",
      no_init)
    .def("load_minimum_spire_client_version", pure_virtual(
      &VirtualDefinitionsClient::LoadMinimumSpireClientVersion))
    .def("load_country_database", pure_virtual(
      &VirtualDefinitionsClient::LoadCountryDatabase))
    .def("load_time_zone_database", pure_virtual(
      &VirtualDefinitionsClient::LoadTimeZoneDatabase))
    .def("load_currency_database", pure_virtual(
      &VirtualDefinitionsClient::LoadCurrencyDatabase))
    .def("load_destination_database", pure_virtual(
      &VirtualDefinitionsClient::LoadDestinationDatabase))
    .def("load_market_database", pure_virtual(
      &VirtualDefinitionsClient::LoadMarketDatabase))
    .def("load_exchange_rates", pure_virtual(
      &VirtualDefinitionsClient::LoadExchangeRates))
    .def("load_compliance_rule_schemas", pure_virtual(
      &VirtualDefinitionsClient::LoadComplianceRuleSchemas))
    .def("open", pure_virtual(&VirtualDefinitionsClient::Open))
    .def("close", pure_virtual(&VirtualDefinitionsClient::Close));
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
