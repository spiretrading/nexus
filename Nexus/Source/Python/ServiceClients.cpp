#include "Nexus/Python/ServiceClients.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/ServiceClients/ApplicationServiceClients.hpp"
#include "Nexus/ServiceClients/TestEnvironment.hpp"
#include "Nexus/ServiceClients/TestServiceClients.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineServiceClients final : VirtualServiceClients {
    ServiceLocatorClient& GetServiceLocatorClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(ServiceLocatorClient&, VirtualServiceClients,
        "get_service_locator_client", GetServiceLocatorClient);
    }

    RegistryClient& GetRegistryClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(RegistryClient&, VirtualServiceClients,
        "get_registry_client", GetRegistryClient);
    }

    AdministrationClient& GetAdministrationClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(AdministrationClient&, VirtualServiceClients,
        "get_administration_client", GetAdministrationClient);
    }

    DefinitionsClient& GetDefinitionsClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(DefinitionsClient&, VirtualServiceClients,
        "get_definitions_client", GetDefinitionsClient);
    }

    MarketDataClient& GetMarketDataClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(MarketDataClient&, VirtualServiceClients,
        "get_market_data_client", GetMarketDataClient);
    }

    ChartingClient& GetChartingClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(ChartingClient&, VirtualServiceClients,
        "get_charting_client", GetChartingClient);
    }

    ComplianceClient& GetComplianceClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(ComplianceClient&, VirtualServiceClients,
        "get_compliance_client", GetComplianceClient);
    }

    OrderExecutionClient& GetOrderExecutionClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(OrderExecutionClient&, VirtualServiceClients,
        "get_order_execution_client", GetOrderExecutionClient);
    }

    RiskClient& GetRiskClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(RiskClient&, VirtualServiceClients,
        "get_risk_client", GetRiskClient);
    }

    TimeClient& GetTimeClient() override {
      PYBIND11_OVERLOAD_PURE_NAME(TimeClient&, VirtualServiceClients,
        "get_time_client", GetTimeClient);
    }

    std::unique_ptr<Timer> BuildTimer(time_duration expiry) override {
      return MakeVirtualTimer(BuildPythonTimer(expiry));
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceClients, "close", Close);
    }

    std::shared_ptr<Timer> BuildPythonTimer(time_duration expiry) {
      PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<Timer>,
        VirtualServiceClients, "build_timer", BuildPythonTimer, expiry);
    }
  };

  struct PythonTestServiceClients final :
      ToPythonServiceClients<TestServiceClients> {
    std::shared_ptr<TestEnvironment> m_environment;

    PythonTestServiceClients(std::unique_ptr<TestServiceClients> serviceClients,
      std::shared_ptr<TestEnvironment> environment)
      : ToPythonServiceClients<TestServiceClients>(std::move(serviceClients)),
        m_environment(std::move(environment)) {}

    ~PythonTestServiceClients() override {
      auto release = gil_scoped_release();
      Close();
      m_environment.reset();
    }
  };
}

void Nexus::Python::ExportApplicationServiceClients(pybind11::module& module) {
  class_<ToPythonServiceClients<ApplicationServiceClients>,
      std::shared_ptr<ToPythonServiceClients<ApplicationServiceClients>>,
      VirtualServiceClients>(module, "ApplicationServiceClients")
    .def(init(
      [] (const IpAddress& address, const std::string& username,
          const std::string& password) {
        return MakeToPythonServiceClients(
          std::make_unique<ApplicationServiceClients>(address, username,
          password, Ref(*GetSocketThreadPool()), Ref(*GetTimerThreadPool())));
      }));
}

void Nexus::Python::ExportServiceClients(pybind11::module& module) {
  ExportVirtualServiceClients(module);
  ExportApplicationServiceClients(module);
  ExportTestEnvironment(module);
  ExportTestServiceClients(module);
}

void Nexus::Python::ExportTestEnvironment(pybind11::module& module) {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>>(module,
      "TestEnvironment")
    .def(init())
    .def(init<std::shared_ptr<VirtualHistoricalDataStore>>())
    .def("set_time", &TestEnvironment::SetTime, call_guard<GilRelease>())
    .def("advance_time", &TestEnvironment::AdvanceTime,
      call_guard<GilRelease>())
    .def("publish", static_cast<void (TestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const BboQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const BookQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const MarketQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const TimeAndSale&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("update_bbo_price", static_cast<void (TestEnvironment::*)(
      const Security&, Money, Money)>(&TestEnvironment::UpdateBboPrice),
      call_guard<GilRelease>())
    .def("update_bbo_price", static_cast<void (TestEnvironment::*)(
      const Security&, Money, Money, const ptime&)>(
      &TestEnvironment::UpdateBboPrice), call_guard<GilRelease>())
    .def("monitor_order_submissions", &TestEnvironment::MonitorOrderSubmissions,
      call_guard<GilRelease>())
    .def("accept", &TestEnvironment::Accept, call_guard<GilRelease>())
    .def("reject", &TestEnvironment::Reject, call_guard<GilRelease>())
    .def("cancel", &TestEnvironment::Cancel, call_guard<GilRelease>())
    .def("fill", static_cast<void (TestEnvironment::*)(const Order&, Money,
      Quantity)>(&TestEnvironment::Fill), call_guard<GilRelease>())
    .def("fill", static_cast<void (TestEnvironment::*)(const Order&, Quantity)>(
      &TestEnvironment::Fill), call_guard<GilRelease>())
    .def("update", static_cast<void (TestEnvironment::*)(const Order&,
      const ExecutionReport&)>(&TestEnvironment::Update),
      call_guard<GilRelease>())
    .def("get_time_environment", &TestEnvironment::GetTimeEnvironment,
      return_value_policy::reference_internal)
    .def("get_service_locator_environment",
      &TestEnvironment::GetServiceLocatorEnvironment,
      return_value_policy::reference_internal)
    .def("get_uid_environment", &TestEnvironment::GetUidEnvironment,
      return_value_policy::reference_internal)
    .def("get_administration_environment",
      &TestEnvironment::GetAdministrationEnvironment,
      return_value_policy::reference_internal)
    .def("get_market_data_environment",
      &TestEnvironment::GetMarketDataEnvironment,
      return_value_policy::reference_internal)
    .def("get_order_execution_environment",
      &TestEnvironment::GetOrderExecutionEnvironment,
      return_value_policy::reference_internal)
    .def("close", &TestEnvironment::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportTestServiceClients(pybind11::module& module) {
  class_<PythonTestServiceClients, std::shared_ptr<PythonTestServiceClients>,
    VirtualServiceClients>(module, "TestServiceClients")
    .def(init(
      [] (std::shared_ptr<TestEnvironment> environment) {
        return std::make_shared<PythonTestServiceClients>(
          std::make_unique<TestServiceClients>(Ref(*environment)), environment);
      }));
}

void Nexus::Python::ExportVirtualServiceClients(pybind11::module& module) {
  class_<VirtualServiceClients, TrampolineServiceClients,
    std::shared_ptr<VirtualServiceClients>>(module, "ServiceClients")
    .def("get_service_locator_client",
      &VirtualServiceClients::GetServiceLocatorClient,
      return_value_policy::reference_internal)
    .def("get_registry_client", &VirtualServiceClients::GetRegistryClient,
      return_value_policy::reference_internal)
    .def("get_administration_client",
      &VirtualServiceClients::GetAdministrationClient,
      return_value_policy::reference_internal)
    .def("get_definitions_client", &VirtualServiceClients::GetDefinitionsClient,
      return_value_policy::reference_internal)
    .def("get_market_data_client", &VirtualServiceClients::GetMarketDataClient,
      return_value_policy::reference_internal)
    .def("get_charting_client", &VirtualServiceClients::GetChartingClient,
      return_value_policy::reference_internal)
    .def("get_compliance_client", &VirtualServiceClients::GetComplianceClient,
      return_value_policy::reference_internal)
    .def("get_order_execution_client",
      &VirtualServiceClients::GetOrderExecutionClient,
      return_value_policy::reference_internal)
    .def("get_risk_client", &VirtualServiceClients::GetRiskClient,
      return_value_policy::reference_internal)
    .def("get_time_client", &VirtualServiceClients::GetTimeClient,
      return_value_policy::reference_internal)
    .def("build_timer",
      [] (VirtualServiceClients& serviceClients, const time_duration& expiry) {
        return std::shared_ptr(serviceClients.BuildTimer(expiry));
      })
    .def("close", &VirtualServiceClients::Close);
}
