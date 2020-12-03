#include "Nexus/Python/DefinitionsService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::DefinitionsService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto definitionsClientBox = std::unique_ptr<class_<DefinitionsClientBox>>();
}

class_<DefinitionsClientBox>& Nexus::Python::GetExportedDefinitionsClientBox() {
  return *definitionsClientBox;
}

void Nexus::Python::ExportApplicationDefinitionsClient(module& module) {
  using PythonApplicationDefinitionsClient = ToPythonDefinitionsClient<
    DefinitionsClient<DefaultSessionBuilder<ServiceLocatorClientBox>>>;
  ExportDefinitionsClient<PythonApplicationDefinitionsClient>(module,
    "ApplicationDefinitionsClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationDefinitionsClient>(
        MakeDefaultSessionBuilder(std::move(serviceLocatorClient),
          DefinitionsService::SERVICE_NAME));
    }));
}

void Nexus::Python::ExportDefinitionsService(module& module) {
  auto submodule = module.def_submodule("definitions_service");
  definitionsClientBox = std::make_unique<class_<DefinitionsClientBox>>(
    ExportDefinitionsClient<DefinitionsClientBox>(submodule,
      "DefinitionsClient"));
  ExportDefinitionsClient<ToPythonDefinitionsClient<DefinitionsClientBox>>(
    submodule, "DefinitionsClientBox");
  ExportApplicationDefinitionsClient(submodule);
  auto test_module = submodule.def_submodule("tests");
  ExportDefinitionsServiceTestEnvironment(test_module);
}

void Nexus::Python::ExportDefinitionsServiceTestEnvironment(module& module) {
  class_<DefinitionsServiceTestEnvironment>(module,
    "DefinitionsServiceTestEnvironment").
    def(init<ServiceLocatorClientBox>(), call_guard<GilRelease>()).
    def("__del__",
      [] (DefinitionsServiceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def("close", &DefinitionsServiceTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("make_client",
      [] (DefinitionsServiceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonDefinitionsClient(self.MakeClient(
          std::move(serviceLocatorClient)));
      }, call_guard<GilRelease>());
}
