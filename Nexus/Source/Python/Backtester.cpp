#include "Nexus/Python/Backtester.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/Python/ServiceClients.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineBacktesterEnvironment final : BacktesterEnvironment {
    std::shared_ptr<VirtualServiceClients> m_serviceClients;

    TrampolineBacktesterEnvironment(ptime startTime,
      std::shared_ptr<VirtualServiceClients> serviceClients)
      : BacktesterEnvironment(startTime, Ref(*serviceClients)),
        m_serviceClients(std::move(serviceClients)) {}

    TrampolineBacktesterEnvironment(ptime startTime, ptime endTime,
      std::shared_ptr<VirtualServiceClients> serviceClients)
      : BacktesterEnvironment{startTime, endTime, Ref(*serviceClients)},
        m_serviceClients(std::move(serviceClients)) {}

    ~TrampolineBacktesterEnvironment() {
      auto release = GilRelease();
      Close();
    }
  };

  struct ToPythonBacktesterServiceClients final :
      ToPythonServiceClients<BacktesterServiceClients> {
    std::shared_ptr<BacktesterEnvironment> m_environment;

    ToPythonBacktesterServiceClients(
      std::shared_ptr<BacktesterEnvironment> environment)
      : ToPythonServiceClients<BacktesterServiceClients>(
          std::make_unique<BacktesterServiceClients>(Ref(*environment))),
        m_environment(std::move(environment)) {}

    ~ToPythonBacktesterServiceClients() override {
      Close();
    }
  };
}

void Nexus::Python::ExportBacktester(module& module) {
  ExportBacktesterEnvironment(module);
  ExportBacktesterEventHandler(module);
  ExportBacktesterServiceClients(module);
}

void Nexus::Python::ExportBacktesterEnvironment(module& module) {
  class_<BacktesterEnvironment, std::shared_ptr<BacktesterEnvironment>>(module,
      "BacktesterEnvironment")
    .def(init(
      [] (ptime startTime,
          std::shared_ptr<VirtualServiceClients> serviceClients) {
        return std::make_unique<TrampolineBacktesterEnvironment>(startTime,
          std::move(serviceClients));
      }), call_guard<GilRelease>())
    .def(init(
      [] (ptime startTime, ptime endTime,
          std::shared_ptr<VirtualServiceClients> serviceClients) {
        return std::make_unique<TrampolineBacktesterEnvironment>(startTime,
          endTime, std::move(serviceClients));
      }), call_guard<GilRelease>())
    .def("__del__",
      [] (BacktesterEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>())
    .def_property_readonly("event_handler",
      static_cast<BacktesterEventHandler& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetEventHandler),
      return_value_policy::reference_internal)
    .def_property_readonly("service_locator_environment",
      &BacktesterEnvironment::GetServiceLocatorEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("uid_environment",
      &BacktesterEnvironment::GetUidEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("registry_environment",
      &BacktesterEnvironment::GetRegistryEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("definitions_environment",
      &BacktesterEnvironment::GetDefinitionsEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("administration_environment",
      &BacktesterEnvironment::GetAdministrationEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("market_data_environment",
      &BacktesterEnvironment::GetMarketDataEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("market_data_service",
      static_cast<BacktesterMarketDataService& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetMarketDataService),
      return_value_policy::reference_internal)
    .def_property_readonly("charting_environment",
      &BacktesterEnvironment::GetChartingEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("compliance_environment",
      &BacktesterEnvironment::GetComplianceEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("order_execution_environment",
      &BacktesterEnvironment::GetOrderExecutionEnvironment,
      return_value_policy::reference_internal)
    .def_property_readonly("risk_environment",
      &BacktesterEnvironment::GetRiskEnvironment,
      return_value_policy::reference_internal)
    .def("close", &BacktesterEnvironment::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportBacktesterEventHandler(module& module) {
  class_<BacktesterEventHandler>(module, "BacktesterEventHandler")
    .def(init<ptime>(), call_guard<GilRelease>())
    .def(init<ptime, ptime>(), call_guard<GilRelease>())
    .def("__del__",
      [] (BacktesterEventHandler& self) {
        self.Close();
      }, call_guard<GilRelease>())
    .def_property_readonly("start_time", &BacktesterEventHandler::GetStartTime)
    .def_property_readonly("end_time", &BacktesterEventHandler::GetEndTime)
    .def("add", static_cast<void (BacktesterEventHandler::*)(
      std::shared_ptr<BacktesterEvent>)>(&BacktesterEventHandler::Add))
    .def("add",
      [] (BacktesterEventHandler& self, const object& events) {
        auto e = std::vector<std::shared_ptr<BacktesterEvent>>();
        for(auto& event : events) {
          e.push_back(event.cast<std::shared_ptr<BacktesterEvent>>());
        }
        self.Add(std::move(e));
      })
    .def("close", &BacktesterEventHandler::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportBacktesterServiceClients(module& module) {
  class_<ToPythonServiceClients<BacktesterServiceClients>,
      std::shared_ptr<ToPythonServiceClients<BacktesterServiceClients>>,
      VirtualServiceClients>(module, "BacktesterServiceClients")
    .def(init(
      [] (std::shared_ptr<BacktesterEnvironment> environment) {
        return std::make_shared<ToPythonBacktesterServiceClients>(
          std::move(environment));
      }), call_guard<GilRelease>());
}
