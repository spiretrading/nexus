#include "Nexus/Python/Backtester.hpp"
#include <Beam/Python/BoostPython.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/Python/ToPythonServiceClients.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace std;

namespace {
  struct ToPythonBacktesterEnvironment : BacktesterEnvironment,
      wrapper<BacktesterEnvironment> {
    std::shared_ptr<VirtualServiceClients> m_serviceClients;

    ToPythonBacktesterEnvironment(const ptime& startTime,
        std::shared_ptr<VirtualServiceClients> serviceClients)
        : BacktesterEnvironment{startTime, Ref(*serviceClients)},
          m_serviceClients{std::move(serviceClients)} {}

    ToPythonBacktesterEnvironment(const ptime& startTime, const ptime& endTime,
        std::shared_ptr<VirtualServiceClients> serviceClients)
        : BacktesterEnvironment{startTime, endTime, Ref(*serviceClients)},
          m_serviceClients{std::move(serviceClients)} {}
  };

  auto MakeBacktesterEventHandlerA(const ptime& startTime) {
    return std::make_shared<BacktesterEventHandler>(startTime);
  }

  auto MakeBacktesterEventHandlerB(const ptime& startTime,
      const ptime& endTime) {
    return std::make_shared<BacktesterEventHandler>(startTime, endTime);
  }

  void BacktesterEventHandlerAddEvents(BacktesterEventHandler& eventHandler,
      const object& events) {
    stl_input_iterator<std::shared_ptr<BacktesterEvent>> begin{events};
    stl_input_iterator<std::shared_ptr<BacktesterEvent>> end;
    vector<std::shared_ptr<BacktesterEvent>> e{begin, end};
    eventHandler.Add(std::move(e));
  }

  auto MakeBacktesterServiceClients(
      std::shared_ptr<BacktesterEnvironment> environment) {
    return MakeToPythonServiceClients(
      std::make_unique<BacktesterServiceClients>(Ref(*environment)));
  }
}

void Nexus::Python::ExportBacktester() {
  ExportBacktesterEnvironment();
  ExportBacktesterEventHandler();
  ExportBacktesterServiceClients();
}

void Nexus::Python::ExportBacktesterEnvironment() {
  class_<ToPythonBacktesterEnvironment,
      std::shared_ptr<ToPythonBacktesterEnvironment>, boost::noncopyable>(
      "BacktesterEnvironment",
      init<const ptime&, std::shared_ptr<VirtualServiceClients>>())
    .def(init<const ptime&, const ptime&,
      std::shared_ptr<VirtualServiceClients>>())
    .add_property("event_handler", make_function(
      static_cast<BacktesterEventHandler& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetEventHandler), return_internal_reference<>()))
    .add_property("market_data_server", make_function(
      static_cast<BacktesterMarketDataService& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetMarketDataService),
      return_internal_reference<>()))
    .def("open", BlockingFunction(&BacktesterEnvironment::Open))
    .def("close", BlockingFunction(&BacktesterEnvironment::Close));
  boost::python::register_ptr_to_python<
    std::shared_ptr<BacktesterEnvironment>>();
  implicitly_convertible<std::shared_ptr<ToPythonBacktesterEnvironment>,
    std::shared_ptr<BacktesterEnvironment>>();
}

void Nexus::Python::ExportBacktesterEventHandler() {
  class_<BacktesterEventHandler, std::shared_ptr<BacktesterEventHandler>,
      boost::noncopyable>("BacktesterEventHandler", no_init)
    .def("__init__", make_constructor(&MakeBacktesterEventHandlerA))
    .def("__init__", make_constructor(&MakeBacktesterEventHandlerB))
    .add_property("start_time", &BacktesterEventHandler::GetStartTime)
    .add_property("end_time", &BacktesterEventHandler::GetEndTime)
    .def("add", static_cast<void (BacktesterEventHandler::*)(
      std::shared_ptr<BacktesterEvent>)>(&BacktesterEventHandler::Add))
    .def("add", &BacktesterEventHandlerAddEvents)
    .def("open", BlockingFunction(&BacktesterEventHandler::Open))
    .def("close", BlockingFunction(&BacktesterEventHandler::Close));
}

void Nexus::Python::ExportBacktesterServiceClients() {
  class_<ToPythonServiceClients<BacktesterServiceClients>, boost::noncopyable,
    bases<VirtualServiceClients>>("BacktesterServiceClients", no_init)
    .def("__init__", make_constructor(&MakeBacktesterServiceClients));
}
