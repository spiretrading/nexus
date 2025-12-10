#include "Nexus/Python/Backtester.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Backtester/ActiveBacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterClients.hpp"
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Python/Clients.hpp"
#include "Nexus/Python/ToPythonClients.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_active_backtester_event(module& module) {
  struct PythonActiveBacktesterEvent : ActiveBacktesterEvent {
    using ActiveBacktesterEvent::ActiveBacktesterEvent;
    void execute() override {
      PYBIND11_OVERRIDE(void, ActiveBacktesterEvent, execute, );
    }
  };
  class_<ActiveBacktesterEvent, PythonActiveBacktesterEvent, BacktesterEvent,
    std::shared_ptr<ActiveBacktesterEvent>>(module, "ActiveBacktesterEvent").
    def(init()).
    def(init<ptime>());
}

void Nexus::Python::export_backtester(module& module) {
  export_backtester_clients(module);
  export_backtester_environment(module);
  export_backtester_event(module);
  export_active_backtester_event(module);
  export_backtester_event_handler(module);
}

void Nexus::Python::export_backtester_clients(module& module) {
  export_clients<ToPythonClients<BacktesterClients>>(
    module, "BacktesterClients").
      def(init([] (BacktesterEnvironment& environment) {
        return std::make_unique<ToPythonClients<BacktesterClients>>(
          Ref(environment));
      }), call_guard<gil_scoped_release>(), keep_alive<1, 2>());
}

void Nexus::Python::export_backtester_environment(module& module) {
  class_<BacktesterEnvironment, std::shared_ptr<BacktesterEnvironment>>(
      module, "BacktesterEnvironment").
    def(init(&make_python_shared<BacktesterEnvironment, ptime, Clients&>),
      keep_alive<1, 3>()).
    def(
      init(&make_python_shared<BacktesterEnvironment, ptime, ptime, Clients&>),
      keep_alive<1, 4>()).
    def_property_readonly("event_handler",
      overload_cast<>(&BacktesterEnvironment::get_event_handler),
      return_value_policy::reference_internal).
    def_property_readonly("market_data_service",
      overload_cast<>(&BacktesterEnvironment::get_market_data_service),
      return_value_policy::reference_internal).
    def("get_service_locator_environment",
      &BacktesterEnvironment::get_service_locator_environment,
      return_value_policy::reference_internal).
    def("get_uid_environment", &BacktesterEnvironment::get_uid_environment,
      return_value_policy::reference_internal).
    def("get_definitions_environment",
      &BacktesterEnvironment::get_definitions_environment,
      return_value_policy::reference_internal).
    def("get_administration_environment",
      &BacktesterEnvironment::get_administration_environment,
      return_value_policy::reference_internal).
    def("get_market_data_environment",
      &BacktesterEnvironment::get_market_data_environment,
      return_value_policy::reference_internal).
    def("get_charting_environment",
      &BacktesterEnvironment::get_charting_environment,
      return_value_policy::reference_internal).
    def("get_compliance_environment",
      &BacktesterEnvironment::get_compliance_environment,
      return_value_policy::reference_internal).
    def("get_order_execution_environment",
      &BacktesterEnvironment::get_order_execution_environment,
      return_value_policy::reference_internal).
    def("get_risk_environment", &BacktesterEnvironment::get_risk_environment,
      return_value_policy::reference_internal).
    def(
      "close", &BacktesterEnvironment::close, call_guard<gil_scoped_release>());
}

void Nexus::Python::export_backtester_event(module& module) {
  struct PythonBacktesterEvent : BacktesterEvent {
    using BacktesterEvent::BacktesterEvent;
    bool is_passive() const override {
      PYBIND11_OVERRIDE(bool, BacktesterEvent, is_passive, );
    }
    void execute() override {
      PYBIND11_OVERRIDE_PURE(void, BacktesterEvent, execute, );
    }
  };
  class_<BacktesterEvent, PythonBacktesterEvent,
    std::shared_ptr<BacktesterEvent>>(module, "BacktesterEvent").
      def("get_timestamp", &BacktesterEvent::get_timestamp).
      def("wait", &BacktesterEvent::wait, call_guard<gil_scoped_release>()).
      def("is_passive", &BacktesterEvent::is_passive).
      def("execute", &BacktesterEvent::execute);
}

void Nexus::Python::export_backtester_event_handler(module& module) {
  class_<BacktesterEventHandler, std::shared_ptr<BacktesterEventHandler>>(
    module, "BacktesterEventHandler").
      def(init(&make_python_shared<BacktesterEventHandler, ptime>)).
      def(init(&make_python_shared<BacktesterEventHandler, ptime, ptime>)).
      def_property_readonly(
        "start_time", &BacktesterEventHandler::get_start_time).
      def_property_readonly("end_time", &BacktesterEventHandler::get_end_time).
      def_property_readonly("time", &BacktesterEventHandler::get_time).
      def("add", overload_cast<const std::shared_ptr<BacktesterEvent>&>(
          &BacktesterEventHandler::add)).
      def("add", [] (BacktesterEventHandler& self, const object& events) {
        auto e = std::vector<std::shared_ptr<BacktesterEvent>>();
        for(auto& event : events) {
          e.push_back(event.cast<std::shared_ptr<BacktesterEvent>>());
        }
        self.add(std::move(e));
      }).
      def("close", &BacktesterEventHandler::close,
        call_guard<gil_scoped_release>());
}
