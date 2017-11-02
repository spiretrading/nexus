#include "Nexus/Python/ServiceClients.hpp"
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/UniquePtr.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Python/ToPythonServiceClients.hpp"
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
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace std;

namespace {
  struct FromPythonServiceClients : VirtualServiceClients,
      wrapper<VirtualServiceClients> {
    virtual ServiceLocatorClient& GetServiceLocatorClient() override final {
      return *static_cast<ServiceLocatorClient*>(
        get_override("get_service_locator_client")());
    }

    virtual RegistryClient& GetRegistryClient() override final {
      return *static_cast<RegistryClient*>(
        get_override("get_registry_client")());
    }

    virtual AdministrationClient& GetAdministrationClient() override final {
      return *static_cast<AdministrationClient*>(
        get_override("get_administration_client")());
    }

    virtual DefinitionsClient& GetDefinitionsClient() override final {
      return *static_cast<DefinitionsClient*>(
        get_override("get_definitions_client")());
    }

    virtual MarketDataClient& GetMarketDataClient() override final {
      return *static_cast<MarketDataClient*>(
        get_override("get_market_data_client")());
    }

    virtual ChartingClient& GetChartingClient() override final {
      return *static_cast<ChartingClient*>(
        get_override("get_charting_client")());
    }

    virtual ComplianceClient& GetComplianceClient() override final {
      return *static_cast<ComplianceClient*>(
        get_override("get_compliance_client")());
    }

    virtual OrderExecutionClient& GetOrderExecutionClient() override final {
      return *static_cast<OrderExecutionClient*>(
        get_override("get_order_execution_client")());
    }

    virtual RiskClient& GetRiskClient() override final {
      return *static_cast<RiskClient*>(get_override("get_risk_client")());
    }

    virtual TimeClient& GetTimeClient() override final {
      return *static_cast<TimeClient*>(get_override("get_time_client")());
    }

    virtual std::unique_ptr<Timer> BuildTimer(
        boost::posix_time::time_duration expiry) override final {
      return std::unique_ptr<Timer>{
        static_cast<Timer*>(get_override("build_timer")(expiry))};
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto BuildApplicationServiceClients(const IpAddress& address,
      const string& username, const string& password) {
    return MakeToPythonServiceClients(
      std::make_unique<ApplicationServiceClients>(address, username, password,
      Ref(*GetSocketThreadPool()), Ref(*GetTimerThreadPool()))).release();
  }

  auto BuildTestServiceClients(std::shared_ptr<TestEnvironment> environment) {
    return MakeToPythonServiceClients(std::make_unique<TestServiceClients>(
      Ref(*environment))).release();
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(ChartingService::VirtualChartingClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Compliance::VirtualComplianceClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(DefinitionsService::VirtualDefinitionsClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualMarketDataClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  OrderExecutionService::VirtualOrderExecutionClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(RegistryService::VirtualRegistryClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(RiskService::VirtualRiskClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ServiceLocator::VirtualServiceLocatorClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualServiceClients);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualTimeClient);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualTimer);

void Nexus::Python::ExportApplicationServiceClients() {
  class_<ToPythonServiceClients<ApplicationServiceClients>,
    boost::noncopyable, bases<VirtualServiceClients>>(
    "ApplicationServiceClients", no_init)
    .def("__init__", make_constructor(&BuildApplicationServiceClients));
}

void Nexus::Python::ExportServiceClients() {
  ExportVirtualServiceClients();
  ExportApplicationServiceClients();
  ExportTestEnvironment();
  ExportTestServiceClients();
}

void Nexus::Python::ExportTestEnvironment() {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>, boost::noncopyable>(
      "TestEnvironment", init<>())
    .def("set_time", BlockingFunction(&TestEnvironment::SetTime))
    .def("advance_time", BlockingFunction(&TestEnvironment::AdvanceTime))
    .def("publish", BlockingFunction(static_cast<void (TestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(static_cast<void (TestEnvironment::*)(
      const Security&, const BboQuote&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, const BookQuote&)>(
      &TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(
      const Security&, const MarketQuote&)>(&TestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (TestEnvironment::*)(
      const Security&, const TimeAndSale&)>(&TestEnvironment::Publish)))
    .def("update_bbo_price", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, Money, Money)>(
      &TestEnvironment::UpdateBboPrice)))
    .def("update_bbo_price", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Security&, Money, Money,
      const ptime&)>(&TestEnvironment::UpdateBboPrice)))
    .def("monitor_order_submissions",
      BlockingFunction(&TestEnvironment::MonitorOrderSubmissions))
    .def("accept_order", BlockingFunction(&TestEnvironment::AcceptOrder))
    .def("reject_order", BlockingFunction(&TestEnvironment::RejectOrder))
    .def("cancel_order", BlockingFunction(&TestEnvironment::CancelOrder))
    .def("fill_order", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&, Money, Quantity)>(
      &TestEnvironment::FillOrder)))
    .def("fill_order", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&, Quantity)>(
      &TestEnvironment::FillOrder)))
    .def("update", BlockingFunction(
      static_cast<void (TestEnvironment::*)(const Order&,
      const ExecutionReport&)>(&TestEnvironment::Update)))
    .def("get_time_environment", &TestEnvironment::GetTimeEnvironment,
      return_internal_reference<>())
    .def("get_service_locator_environment",
      &TestEnvironment::GetServiceLocatorEnvironment,
      return_internal_reference<>())
    .def("get_uid_environment", &TestEnvironment::GetUidEnvironment,
      return_internal_reference<>())
    .def("get_administration_environment",
      &TestEnvironment::GetAdministrationEnvironment,
      return_internal_reference<>())
    .def("get_market_data_environment",
      &TestEnvironment::GetMarketDataEnvironment, return_internal_reference<>())
    .def("get_order_execution_environment",
      &TestEnvironment::GetOrderExecutionEnvironment,
      return_internal_reference<>())
    .def("open", BlockingFunction(&TestEnvironment::Open))
    .def("close", BlockingFunction(&TestEnvironment::Close));
}

void Nexus::Python::ExportTestServiceClients() {
  class_<ToPythonServiceClients<TestServiceClients>, boost::noncopyable,
    bases<VirtualServiceClients>>("TestServiceClients", no_init)
    .def("__init__", make_constructor(&BuildTestServiceClients));
}

void Nexus::Python::ExportVirtualServiceClients() {
  class_<FromPythonServiceClients, boost::noncopyable>("ServiceClients",
    no_init)
    .def("get_service_locator_client",
      pure_virtual(&VirtualServiceClients::GetServiceLocatorClient),
      return_internal_reference<>())
    .def("get_registry_client",
      pure_virtual(&VirtualServiceClients::GetRegistryClient),
      return_internal_reference<>())
    .def("get_administration_client",
      pure_virtual(&VirtualServiceClients::GetAdministrationClient),
      return_internal_reference<>())
    .def("get_definitions_client",
      pure_virtual(&VirtualServiceClients::GetDefinitionsClient),
      return_internal_reference<>())
    .def("get_market_data_client",
      pure_virtual(&VirtualServiceClients::GetMarketDataClient),
      return_internal_reference<>())
    .def("get_charting_client",
      pure_virtual(&VirtualServiceClients::GetChartingClient),
      return_internal_reference<>())
    .def("get_compliance_client",
      pure_virtual(&VirtualServiceClients::GetComplianceClient),
      return_internal_reference<>())
    .def("get_order_execution_client",
      pure_virtual(&VirtualServiceClients::GetOrderExecutionClient),
      return_internal_reference<>())
    .def("get_risk_client", pure_virtual(&VirtualServiceClients::GetRiskClient),
      return_internal_reference<>())
    .def("get_time_client", pure_virtual(&VirtualServiceClients::GetTimeClient),
      return_internal_reference<>())
    .def("build_timer", pure_virtual(&VirtualServiceClients::BuildTimer))
    .def("open", pure_virtual(&VirtualServiceClients::Open))
    .def("close", pure_virtual(&VirtualServiceClients::Close));
  ExportUniquePtr<VirtualServiceClients>();
}
