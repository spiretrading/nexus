#include "Nexus/Python/TestEnvironment.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/Python/Clients.hpp"
#include "Nexus/Python/ToPythonClients.hpp"
#include "Nexus/TestEnvironment/TestClients.hpp"
#include "Nexus/TestEnvironment/TestEnvironmentException.hpp"
#include "Nexus/TestEnvironment/TestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_test_clients(module& module) {
  export_clients<ToPythonClients<TestClients>>(module, "TestClients").
    def(init([] (TestEnvironment& environment) {
      return std::make_unique<ToPythonClients<TestClients>>(Ref(environment));
  }), call_guard<GilRelease>(), keep_alive<1, 2>());
}

void Nexus::Python::export_test_environment(module& module) {
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>>(
    module, "TestEnvironment").
      def(init(&make_python_shared<TestEnvironment>)).
      def(init(&make_python_shared<TestEnvironment, ptime>)).
      def(init(&make_python_shared<TestEnvironment, HistoricalDataStore&>),
        keep_alive<1, 2>()).
      def(init(&make_python_shared<
        TestEnvironment, HistoricalDataStore&, ptime>), keep_alive<1, 2>()).
      def("set_time", &TestEnvironment::set, call_guard<GilRelease>()).
      def("advance_time", &TestEnvironment::advance, call_guard<GilRelease>()).
      def("publish", overload_cast<Venue, const OrderImbalance&>(
        &TestEnvironment::publish), call_guard<GilRelease>()).
      def("publish", overload_cast<const Security&, const BboQuote&>(
        &TestEnvironment::publish), call_guard<GilRelease>()).
      def("publish", overload_cast<const Security&, const BookQuote&>(
        &TestEnvironment::publish), call_guard<GilRelease>()).
      def("publish", overload_cast<const Security&, const TimeAndSale&>(
        &TestEnvironment::publish), call_guard<GilRelease>()).
      def("update_bbo_price", overload_cast<const Security&, Money, Money,
        ptime>(&TestEnvironment::update_bbo_price), call_guard<GilRelease>()).
      def("update_bbo_price", overload_cast<const Security&, Money, Money>(
        &TestEnvironment::update_bbo_price), call_guard<GilRelease>()).
      def("monitor_order_submissions",
        &TestEnvironment::monitor_order_submissions, call_guard<GilRelease>()).
      def("accept", &TestEnvironment::accept, call_guard<GilRelease>()).
      def("reject", &TestEnvironment::reject, call_guard<GilRelease>()).
      def("cancel", &TestEnvironment::cancel, call_guard<GilRelease>()).
      def("fill", overload_cast<const Order&, Money, Quantity>(
        &TestEnvironment::fill), call_guard<GilRelease>()).
      def("fill", overload_cast<const Order&, Quantity>(&TestEnvironment::fill),
        call_guard<GilRelease>()).
      def("update", &TestEnvironment::update, call_guard<GilRelease>()).
      def("get_time_environment", &TestEnvironment::get_time_environment,
        return_value_policy::reference_internal).
      def("get_service_locator_environment",
        &TestEnvironment::get_service_locator_environment,
        return_value_policy::reference_internal).
      def("get_uid_environment", &TestEnvironment::get_uid_environment,
        return_value_policy::reference_internal).
      def("get_definitions_environment",
        &TestEnvironment::get_definitions_environment,
        return_value_policy::reference_internal).
      def("get_administration_environment",
        &TestEnvironment::get_administration_environment,
        return_value_policy::reference_internal).
      def("get_market_data_environment",
        &TestEnvironment::get_market_data_environment,
        return_value_policy::reference_internal).
      def("get_charting_environment",
        &TestEnvironment::get_charting_environment,
        return_value_policy::reference_internal).
      def("get_compliance_environment",
        &TestEnvironment::get_compliance_environment,
        return_value_policy::reference_internal).
      def("get_order_execution_environment",
        &TestEnvironment::get_order_execution_environment,
        return_value_policy::reference_internal).
      def("get_risk_environment", &TestEnvironment::get_risk_environment,
        return_value_policy::reference_internal).
      def("close", &TestEnvironment::close, call_guard<GilRelease>());
  export_test_clients(module);
  export_test_environment_exception(module);
}

void Nexus::Python::export_test_environment_exception(module& module) {
  register_exception<TestEnvironmentException>(
    module, "TestEnvironmentException");
}
