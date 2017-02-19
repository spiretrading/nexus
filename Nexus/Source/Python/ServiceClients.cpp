#include "Nexus/Python/ServiceClients.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Python/PythonMarketDataClient.hpp"
#include "Nexus/Python/PythonOrderExecutionClient.hpp"
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace std;

namespace {
  struct VirtualServiceClientsWrapper :
      VirtualServiceClients, wrapper<VirtualServiceClients> {
    virtual ServiceLocatorClient& GetServiceLocatorClient() override {
      return *static_cast<ServiceLocatorClient*>(
        this->get_override("get_service_locator_client")());
    }

    virtual RegistryClient& GetRegistryClient() override {
      return *static_cast<RegistryClient*>(
        this->get_override("get_registry_client")());
    }

    virtual AdministrationClient& GetAdministrationClient() override {
      return *static_cast<AdministrationClient*>(
        this->get_override("get_administration_client")());
    }

    virtual DefinitionsClient& GetDefinitionsClient() override {
      return *static_cast<DefinitionsClient*>(
        this->get_override("get_definitions_client")());
    }

    virtual MarketDataClient& GetMarketDataClient() override {
      return *static_cast<MarketDataClient*>(
        this->get_override("get_market_data_client")());
    }

    virtual ChartingClient& GetChartingClient() override {
      return *static_cast<ChartingClient*>(
        this->get_override("get_charting_client")());
    }

    virtual ComplianceClient& GetComplianceClient() override {
      return *static_cast<ComplianceClient*>(
        this->get_override("get_compliance_client")());
    }

    virtual OrderExecutionClient& GetOrderExecutionClient() override {
      return *static_cast<OrderExecutionClient*>(
        this->get_override("get_order_execution_client")());
    }

    virtual RiskClient& GetRiskClient() override {
      return *static_cast<RiskClient*>(this->get_override("get_risk_client")());
    }

    virtual TimeClient& GetTimeClient() override {
      return *static_cast<TimeClient*>(this->get_override("get_time_client")());
    }

    virtual void Open() override {
      this->get_override("open")();
    }

    virtual void Close() override {
      this->get_override("close")();
    }
  };

  class PythonApplicationServiceClients :
      public WrapperServiceClients<std::unique_ptr<VirtualServiceClients>> {
    public:
      PythonApplicationServiceClients(
          std::unique_ptr<VirtualServiceClients> client)
          : WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>(
              std::move(client)) {}

      virtual PythonMarketDataClient& GetMarketDataClient() override {
        if(m_marketDataClient == nullptr) {
          m_marketDataClient = std::make_unique<PythonMarketDataClient>(
            MakeVirtualMarketDataClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetMarketDataClient()));
        }
        return *m_marketDataClient;
      }

      virtual PythonOrderExecutionClient& GetOrderExecutionClient() override {
        if(m_orderExecutionClient == nullptr) {
          m_orderExecutionClient = std::make_unique<PythonOrderExecutionClient>(
            MakeVirtualOrderExecutionClient(
            &WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>::
            GetOrderExecutionClient()));
        }
        return *m_orderExecutionClient;
      }

    private:
      std::unique_ptr<PythonMarketDataClient> m_marketDataClient;
      std::unique_ptr<PythonOrderExecutionClient> m_orderExecutionClient;
  };

  VirtualServiceClients* BuildApplicationServiceClients(
      const IpAddress& address, const string& username,
      const string& password) {
    auto baseClient = std::make_unique<ApplicationServiceClients>(
      address, username, password, Ref(*GetSocketThreadPool()),
      Ref(*GetTimerThreadPool()));
    return new PythonApplicationServiceClients{
      MakeVirtualServiceClients(std::move(baseClient))};
  }

  class PythonTestServiceClients :
      public WrapperServiceClients<std::unique_ptr<VirtualServiceClients>> {
    public:
      PythonTestServiceClients(std::unique_ptr<VirtualServiceClients> client,
          std::shared_ptr<TestEnvironment> environment)
          : WrapperServiceClients<std::unique_ptr<VirtualServiceClients>>(
              std::move(client)),
            m_environment{std::move(environment)} {}

    private:
      std::shared_ptr<TestEnvironment> m_environment;
  };

  VirtualServiceClients* BuildTestServiceClients(
      std::shared_ptr<TestEnvironment> environment) {
    auto baseClient = std::make_unique<TestServiceClients>(Ref(*environment));
    return new PythonTestServiceClients{
      MakeVirtualServiceClients(std::move(baseClient)), environment};
  }
}

void Nexus::Python::ExportApplicationServiceClients() {
  class_<PythonApplicationServiceClients, boost::noncopyable,
      bases<VirtualServiceClients>>("ApplicationServiceClients", no_init)
    .def("__init__", make_constructor(&BuildApplicationServiceClients))
    .def("get_service_locator_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetServiceLocatorClient,
      return_value_policy<reference_existing_object>()))
    .def("get_registry_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetRegistryClient,
      return_value_policy<reference_existing_object>()))
    .def("get_administration_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetAdministrationClient,
      return_value_policy<reference_existing_object>()))
    .def("get_definitions_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetDefinitionsClient,
      return_value_policy<reference_existing_object>()))
    .def("get_market_data_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetMarketDataClient,
      return_value_policy<reference_existing_object>()))
    .def("get_charting_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetChartingClient,
      return_value_policy<reference_existing_object>()))
    .def("get_compliance_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetComplianceClient,
      return_value_policy<reference_existing_object>()))
    .def("get_order_execution_client",
      BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetOrderExecutionClient,
      return_value_policy<reference_existing_object>()))
    .def("get_risk_client", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetRiskClient,
      return_value_policy<reference_existing_object>()))
    .def("get_time_client", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::GetTimeClient,
      return_value_policy<reference_existing_object>()))
    .def("open", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::Open))
    .def("close", BlockingFunction<PythonApplicationServiceClients>(
      &PythonApplicationServiceClients::Close));
}

void Nexus::Python::ExportServiceClients() {
  ExportVirtualServiceClients();
  ExportApplicationServiceClients();
  ExportTestEnvironment();
  ExportTestServiceClients();
}

void Nexus::Python::ExportTestEnvironment() {
  class_<TestEnvironment, boost::noncopyable>("TestEnvironment", init<>())
    .def("open", BlockingFunction(&TestEnvironment::Open))
    .def("close", BlockingFunction(&TestEnvironment::Close));
}

void Nexus::Python::ExportTestServiceClients() {
  class_<PythonTestServiceClients, boost::noncopyable,
      bases<VirtualServiceClients>>("TestClients", no_init)
    .def("__init__", make_constructor(&BuildTestServiceClients))
    .def("get_service_locator_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetServiceLocatorClient,
      return_value_policy<reference_existing_object>()))
    .def("get_registry_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetRegistryClient,
      return_value_policy<reference_existing_object>()))
    .def("get_administration_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetAdministrationClient,
      return_value_policy<reference_existing_object>()))
    .def("get_definitions_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetDefinitionsClient,
      return_value_policy<reference_existing_object>()))
    .def("get_market_data_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetMarketDataClient,
      return_value_policy<reference_existing_object>()))
    .def("get_charting_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetChartingClient,
      return_value_policy<reference_existing_object>()))
    .def("get_compliance_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetComplianceClient,
      return_value_policy<reference_existing_object>()))
    .def("get_order_execution_client",
      BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetOrderExecutionClient,
      return_value_policy<reference_existing_object>()))
    .def("get_risk_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetRiskClient,
      return_value_policy<reference_existing_object>()))
    .def("get_time_client", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::GetTimeClient,
      return_value_policy<reference_existing_object>()))
    .def("open", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::Open))
    .def("close", BlockingFunction<PythonTestServiceClients>(
      &PythonTestServiceClients::Close));
}

void Nexus::Python::ExportVirtualServiceClients() {
  class_<VirtualServiceClientsWrapper, boost::noncopyable>("ServiceClients")
    .def("get_service_locator_client",
      pure_virtual(&VirtualServiceClients::GetServiceLocatorClient),
      return_value_policy<reference_existing_object>())
    .def("get_registry_client",
      pure_virtual(&VirtualServiceClients::GetRegistryClient),
      return_value_policy<reference_existing_object>())
    .def("get_administration_client",
      pure_virtual(&VirtualServiceClients::GetAdministrationClient),
      return_value_policy<reference_existing_object>())
    .def("get_definitions_client",
      pure_virtual(&VirtualServiceClients::GetDefinitionsClient),
      return_value_policy<reference_existing_object>())
    .def("get_market_data_client",
      pure_virtual(&VirtualServiceClients::GetMarketDataClient),
      return_value_policy<reference_existing_object>())
    .def("get_charting_client",
      pure_virtual(&VirtualServiceClients::GetChartingClient),
      return_value_policy<reference_existing_object>())
    .def("get_compliance_client",
      pure_virtual(&VirtualServiceClients::GetComplianceClient),
      return_value_policy<reference_existing_object>())
    .def("get_order_execution_client",
      pure_virtual(&VirtualServiceClients::GetOrderExecutionClient),
      return_value_policy<reference_existing_object>())
    .def("get_risk_client", pure_virtual(&VirtualServiceClients::GetRiskClient),
      return_value_policy<reference_existing_object>())
    .def("get_time_client", pure_virtual(&VirtualServiceClients::GetTimeClient),
      return_value_policy<reference_existing_object>())
    .def("open", pure_virtual(&VirtualServiceClients::Open))
    .def("close", pure_virtual(&VirtualServiceClients::Close));
}
