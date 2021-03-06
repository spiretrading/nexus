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
  auto serviceClientsBox = std::unique_ptr<class_<ServiceClientsBox>>();

  struct PythonTestServiceClients : ToPythonServiceClients<TestServiceClients> {
    std::shared_ptr<TestEnvironment> m_environment;

    PythonTestServiceClients(std::shared_ptr<TestEnvironment> environment)
      : ToPythonServiceClients<TestServiceClients>(Ref(*environment)),
        m_environment(std::move(environment)) {}

    ~PythonTestServiceClients() {
      auto release = GilRelease();
      Close();
      m_environment.reset();
    }
  };
}

class_<ServiceClientsBox>& Nexus::Python::GetExportedServiceClientsBox() {
  return *serviceClientsBox;
}

void Nexus::Python::ExportApplicationServiceClients(module& module) {
  ExportServiceClients<ToPythonServiceClients<ApplicationServiceClients>>(
    module, "ApplicationServiceClients").
    def(init<std::string, std::string, const IpAddress&>());
}

void Nexus::Python::ExportServiceClients(module& module) {
  serviceClientsBox = std::make_unique<class_<ServiceClientsBox>>(
    ExportServiceClients<ServiceClientsBox>(module, "ServiceClients"));
  ExportServiceClients<ToPythonServiceClients<ServiceClientsBox>>(module,
    "ServiceClientsBox");
  ExportApplicationServiceClients(module);
  ExportTestEnvironment(module);
  ExportTestServiceClients(module);
}

void Nexus::Python::ExportTestEnvironment(module& module) {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>>(module,
      "TestEnvironment").
    def(init<>(), call_guard<GilRelease>()).
    def(init<ptime>(), call_guard<GilRelease>()).
    def(init<HistoricalDataStoreBox>(), call_guard<GilRelease>()).
    def(init<HistoricalDataStoreBox, ptime>(), call_guard<GilRelease>()).
    def("__del__",
      [] (TestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def("set_time", &TestEnvironment::SetTime, call_guard<GilRelease>()).
    def("advance_time", &TestEnvironment::AdvanceTime,
      call_guard<GilRelease>()).
    def("publish", static_cast<void (TestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>()).
    def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const BboQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>()).
    def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const BookQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>()).
    def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const MarketQuote&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>()).
    def("publish", static_cast<void (TestEnvironment::*)(
      const Security&, const TimeAndSale&)>(&TestEnvironment::Publish),
      call_guard<GilRelease>()).
    def("update_bbo_price", static_cast<void (TestEnvironment::*)(
      const Security&, Money, Money)>(&TestEnvironment::UpdateBboPrice),
      call_guard<GilRelease>()).
    def("update_bbo_price", static_cast<void (TestEnvironment::*)(
      const Security&, Money, Money, ptime)>(&TestEnvironment::UpdateBboPrice),
      call_guard<GilRelease>()).
    def("monitor_order_submissions", &TestEnvironment::MonitorOrderSubmissions,
      call_guard<GilRelease>()).
    def("accept", &TestEnvironment::Accept, call_guard<GilRelease>()).
    def("reject", &TestEnvironment::Reject, call_guard<GilRelease>()).
    def("cancel", &TestEnvironment::Cancel, call_guard<GilRelease>()).
    def("fill", static_cast<void (TestEnvironment::*)(const Order&, Money,
      Quantity)>(&TestEnvironment::Fill), call_guard<GilRelease>()).
    def("fill", static_cast<void (TestEnvironment::*)(const Order&, Quantity)>(
      &TestEnvironment::Fill), call_guard<GilRelease>()).
    def("update", static_cast<void (TestEnvironment::*)(const Order&,
      const ExecutionReport&)>(&TestEnvironment::Update),
      call_guard<GilRelease>()).
    def("get_time_environment", &TestEnvironment::GetTimeEnvironment,
      return_value_policy::reference_internal).
    def("get_service_locator_environment",
      &TestEnvironment::GetServiceLocatorEnvironment,
      return_value_policy::reference_internal).
    def("get_uid_environment", &TestEnvironment::GetUidEnvironment,
      return_value_policy::reference_internal).
    def("get_administration_environment",
      &TestEnvironment::GetAdministrationEnvironment,
      return_value_policy::reference_internal).
    def("get_market_data_environment",
      &TestEnvironment::GetMarketDataEnvironment,
      return_value_policy::reference_internal).
    def("get_charting_environment", &TestEnvironment::GetChartingEnvironment,
      return_value_policy::reference_internal).
    def("get_compliance_environment",
      &TestEnvironment::GetComplianceEnvironment,
      return_value_policy::reference_internal).
    def("get_order_execution_environment",
      &TestEnvironment::GetOrderExecutionEnvironment,
      return_value_policy::reference_internal).
    def("get_risk_environment", &TestEnvironment::GetRiskEnvironment,
      return_value_policy::reference_internal).
    def("close", &TestEnvironment::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportTestServiceClients(module& module) {
  ExportServiceClients<PythonTestServiceClients>(module, "TestServiceClients").
    def(init<std::shared_ptr<TestEnvironment>>());
}
