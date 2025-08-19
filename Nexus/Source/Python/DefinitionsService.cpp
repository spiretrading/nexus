#include "Nexus/Python/DefinitionsService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::DefinitionsService;
using namespace Nexus::DefinitionsService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_definitions_service(module& module) {
  auto submodule = module.def_submodule("definitions_service");
  export_definitions_client<ToPythonDefinitionsClient<DefinitionsClient>>(
    submodule, "DefinitionsClient");
  auto test_module = submodule.def_submodule("tests");
  export_definitions_service_test_environment(test_module);
}

void Nexus::Python::export_definitions_service_test_environment(
    module& module) {
  class_<DefinitionsServiceTestEnvironment>(
      module, "DefinitionsServiceTestEnvironment").
    def(init<ServiceLocatorClientBox>(), call_guard<GilRelease>()).
    def("__del__", [] (DefinitionsServiceTestEnvironment& self) {
      self.close();
    }, call_guard<GilRelease>()).
    def("close", &DefinitionsServiceTestEnvironment::close,
      call_guard<GilRelease>()).
    def("make_client", [] (DefinitionsServiceTestEnvironment& self,
        ServiceLocatorClientBox client) {
      return ToPythonDefinitionsClient<DefinitionsClient>(
        self.make_client(std::move(client)));
    }, call_guard<GilRelease>(), pybind11::arg("service_locator_client"));
}
