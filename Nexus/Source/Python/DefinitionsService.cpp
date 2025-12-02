#include "Nexus/Python/DefinitionsService.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/DefinitionsService/ApplicationDefinitions.hpp"
#include "Nexus/DefinitionsService/DefaultDefinitionsClient.hpp"
#include "Nexus/DefinitionsServiceTests/DefinitionsServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonDefinitionsClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto definitions_client = std::unique_ptr<class_<DefinitionsClient>>();
}

class_<DefinitionsClient>& Nexus::Python::get_exported_definitions_client() {
  return *definitions_client;
}

void Nexus::Python::export_default_definitions_client(module& module) {
  export_definitions_client<DefaultDefinitionsClient>(
    module, "DefaultDefinitionsClient");
}

void Nexus::Python::export_definitions_service(module& module) {
  definitions_client = std::make_unique<class_<DefinitionsClient>>(
    export_definitions_client<DefinitionsClient>(module, "DefinitionsClient"));
  export_default_definitions_client(module);
  export_definitions_service_application_definitions(module);
  auto test_module = module.def_submodule("tests");
  export_definitions_service_test_environment(test_module);
}

void Nexus::Python::export_definitions_service_application_definitions(
    module& module) {
  export_definitions_client<
    ToPythonDefinitionsClient<ApplicationDefinitionsClient>>(
      module, "ApplicationDefinitionsClient").
    def(pybind11::init([] (
        ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>& client) {
      return std::make_unique<ToPythonDefinitionsClient<
        ApplicationDefinitionsClient>>(Ref(*client));
    }), keep_alive<1, 2>());
}

void Nexus::Python::export_definitions_service_test_environment(
    module& module) {
  class_<DefinitionsServiceTestEnvironment,
    std::shared_ptr<DefinitionsServiceTestEnvironment>>(
    module, "DefinitionsServiceTestEnvironment").
      def(pybind11::init(&make_python_shared<
        DefinitionsServiceTestEnvironment, ServiceLocatorClient&>),
        keep_alive<1, 2>()).
      def("make_client", [] (DefinitionsServiceTestEnvironment& self,
          ServiceLocatorClient& client) {
        return ToPythonDefinitionsClient(self.make_client(Ref(client)));
      }, call_guard<GilRelease>(), keep_alive<0, 2>()).
      def("close", &DefinitionsServiceTestEnvironment::close,
        call_guard<GilRelease>());
}
