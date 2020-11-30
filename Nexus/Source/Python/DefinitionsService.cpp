#include "Nexus/Python/DefinitionsService.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClientBox.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/Python/DefinitionsClient.hpp"

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
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineDefinitionsClient final : VirtualDefinitionsClient {
    std::string LoadMinimumSpireClientVersion() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::string, VirtualDefinitionsClient,
        "load_minimum_spire_client_version", LoadMinimumSpireClientVersion);
    }

    std::string LoadOrganizationName() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::string, VirtualDefinitionsClient,
        "load_organization_name", LoadOrganizationName);
    }

    CountryDatabase LoadCountryDatabase() override {
      PYBIND11_OVERLOAD_PURE_NAME(CountryDatabase, VirtualDefinitionsClient,
        "load_country_database", LoadCountryDatabase);
    }

    boost::local_time::tz_database LoadTimeZoneDatabase() override {
      PYBIND11_OVERLOAD_PURE_NAME(boost::local_time::tz_database,
        VirtualDefinitionsClient, "load_time_zone_database",
        LoadTimeZoneDatabase);
    }

    CurrencyDatabase LoadCurrencyDatabase() override {
      PYBIND11_OVERLOAD_PURE_NAME(CurrencyDatabase, VirtualDefinitionsClient,
        "load_currency_database", LoadCurrencyDatabase);
    }

    DestinationDatabase LoadDestinationDatabase() override {
      PYBIND11_OVERLOAD_PURE_NAME(DestinationDatabase, VirtualDefinitionsClient,
        "load_destination_database", LoadDestinationDatabase);
    }

    MarketDatabase LoadMarketDatabase() override {
      PYBIND11_OVERLOAD_PURE_NAME(MarketDatabase, VirtualDefinitionsClient,
        "load_market_database", LoadMarketDatabase);
    }

    std::vector<ExchangeRate> LoadExchangeRates() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<ExchangeRate>,
        VirtualDefinitionsClient, "load_exchange_rates", LoadExchangeRates);
    }

    std::vector<Compliance::ComplianceRuleSchema>
        LoadComplianceRuleSchemas() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<Compliance::ComplianceRuleSchema>,
        VirtualDefinitionsClient, "load_compliance_rule_schemas",
        LoadComplianceRuleSchemas);
    }

    TradingSchedule LoadTradingSchedule() override {
      PYBIND11_OVERLOAD_PURE_NAME(TradingSchedule, VirtualDefinitionsClient,
        "load_trading_schedule", LoadTradingSchedule);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualDefinitionsClient, "close",
        Close);
    }
  };
}

void Nexus::Python::ExportApplicationDefinitionsClient(
    pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    ServiceLocatorClientBox, MessageProtocol<std::unique_ptr<TcpSocketChannel>,
      BinarySender<SharedBuffer>, NullEncoder>, LiveTimer>;
  using Client = DefinitionsClient<SessionBuilder>;
  class_<ToPythonDefinitionsClient<Client>, VirtualDefinitionsClient>(module,
      "ApplicationDefinitionsClient")
    .def(init(
      [] (ServiceLocatorClientBox serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          DefinitionsService::SERVICE_NAME);
        auto sessionBuilder = SessionBuilder(serviceLocatorClient,
          [=] {
            return std::make_unique<TcpSocketChannel>(addresses);
          },
          [] {
            return std::make_unique<LiveTimer>(seconds(10));
          });
        return MakeToPythonDefinitionsClient(std::make_unique<Client>(
          sessionBuilder));
      }), call_guard<GilRelease>());
}

void Nexus::Python::ExportDefinitionsClient(pybind11::module& module) {
  class_<VirtualDefinitionsClient, TrampolineDefinitionsClient>(module,
      "DefinitionsClient")
    .def("load_minimum_spire_client_version",
      &VirtualDefinitionsClient::LoadMinimumSpireClientVersion)
    .def("load_organization_name",
      &VirtualDefinitionsClient::LoadOrganizationName)
    .def("load_country_database",
      &VirtualDefinitionsClient::LoadCountryDatabase)
    .def("load_time_zone_database",
      &VirtualDefinitionsClient::LoadTimeZoneDatabase)
    .def("load_currency_database",
      &VirtualDefinitionsClient::LoadCurrencyDatabase)
    .def("load_destination_database",
      &VirtualDefinitionsClient::LoadDestinationDatabase)
    .def("load_market_database",
      &VirtualDefinitionsClient::LoadMarketDatabase)
    .def("load_exchange_rates",
      &VirtualDefinitionsClient::LoadExchangeRates)
    .def("load_compliance_rule_schemas",
      &VirtualDefinitionsClient::LoadComplianceRuleSchemas)
    .def("load_trading_schedule",
      &VirtualDefinitionsClient::LoadTradingSchedule)
    .def("close", &VirtualDefinitionsClient::Close);
}

void Nexus::Python::ExportDefinitionsService(pybind11::module& module) {
  auto submodule = module.def_submodule("definitions_service");
  ExportDefinitionsClient(submodule);
  ExportApplicationDefinitionsClient(submodule);
}
