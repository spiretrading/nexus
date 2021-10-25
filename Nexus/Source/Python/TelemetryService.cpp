#include "Nexus/Python/TelemetryService.hpp"
#include <Beam/IO/ConnectException.hpp>
#include <Beam/Python/Beam.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/TelemetryService/ApplicationDefinitions.hpp"
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"
#include "Nexus/Python/ToPythonTelemetryClient.hpp"
#include "Nexus/Python/ToPythonTelemetryDataStore.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::TelemetryService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto telemetryClientBox = std::unique_ptr<class_<TelemetryClientBox>>();
  auto telemetryDataStoreBox = std::unique_ptr<class_<TelemetryDataStoreBox>>();
}

class_<TelemetryClientBox>& Nexus::Python::GetExportedTelemetryClientBox() {
  return *telemetryClientBox;
}

class_<TelemetryDataStoreBox>&
    Nexus::Python::GetExportedTelemetryDataStoreBox() {
  return *telemetryDataStoreBox;
}

void Nexus::Python::ExportApplicationTelemetryClient(module& module) {
  using PythonApplicationTelemetryClient = ToPythonTelemetryClient<
    TelemetryClient<ZLibSessionBuilder<ServiceLocatorClientBox>>>;
  ExportTelemetryClient<PythonApplicationTelemetryClient>(module,
    "ApplicationTelemetryClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationTelemetryClient>(
        MakeSessionBuilder<ZLibSessionBuilder<ServiceLocatorClientBox>>(
          std::move(serviceLocatorClient), TelemetryService::SERVICE_NAME));
    }));
}

void Nexus::Python::ExportLocalTelemetryDataStore(module& module) {
  ExportTelemetryDataStore<LocalTelemetryDataStore>(
      module, "LocalTelemetryDataStore").
    def(init()).
    def("load_telemetry_events", static_cast<
      std::vector<SequencedAccountTelemetryEvent> (
        LocalTelemetryDataStore::*)() const>(
          &LocalTelemetryDataStore::LoadTelemetryEvents));
}

void Nexus::Python::ExportTelemetryEvent(module& module) {
  class_<TelemetryEvent>(module, "TelemetryEvent").
    def(init()).
    def(init<const TelemetryEvent&>()).
    def_readwrite("session_id", &TelemetryEvent::m_sessionId).
    def_readwrite("name", &TelemetryEvent::m_name).
    def_readwrite("timestamp", &TelemetryEvent::m_timestamp).
    def_readwrite("data", &TelemetryEvent::m_data).
    def("__str__", &lexical_cast<std::string, TelemetryEvent>);
  ExportQueueSuite<TelemetryEvent>(module, "TelemetryEvent");
  ExportQueueSuite<SequencedTelemetryEvent>(module, "SequencedTelemetryEvent");
}

void Nexus::Python::ExportTelemetryService(module& module) {
  auto submodule = module.def_submodule("telemetry_service");
  telemetryClientBox = std::make_unique<class_<TelemetryClientBox>>(
    ExportTelemetryClient<TelemetryClientBox>(submodule, "TelemetryClient"));
  ExportTelemetryClient<ToPythonTelemetryClient<TelemetryClientBox>>(
    submodule, "TelemetryClientBox");
  ExportApplicationTelemetryClient(submodule);
  ExportLocalTelemetryDataStore(submodule);
  ExportTelemetryEvent(submodule);
  auto testModule = submodule.def_submodule("tests");
  ExportTelemetryServiceTestEnvironment(testModule);
}

void Nexus::Python::ExportTelemetryServiceTestEnvironment(module& module) {}
