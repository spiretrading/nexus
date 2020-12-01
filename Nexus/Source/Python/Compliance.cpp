#include "Nexus/Python/Compliance.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/ComplianceClientBox.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto complianceClientBox = std::unique_ptr<class_<ComplianceClientBox>>();
}

class_<ComplianceClientBox>& Nexus::Python::GetExportedComplianceClientBox() {
  return *complianceClientBox;
}

void Nexus::Python::ExportApplicationComplianceClient(module& module) {
  using PythonApplicationComplianceClient = ToPythonComplianceClient<
    ComplianceClient<DefaultSessionBuilder<ServiceLocatorClientBox>>>;
  ExportComplianceClient<PythonApplicationComplianceClient>(module,
    "ApplicationComplianceClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationComplianceClient>(
        MakeDefaultSessionBuilder(std::move(serviceLocatorClient),
          Compliance::SERVICE_NAME));
    }), call_guard<GilRelease>());
}

void Nexus::Python::ExportCompliance(module& module) {
  auto submodule = module.def_submodule("compliance");
  complianceClientBox = std::make_unique<class_<ComplianceClientBox>>(
    ExportComplianceClient<ComplianceClientBox>(submodule, "ComplianceClient"));
  ExportComplianceClient<ToPythonComplianceClient<ComplianceClientBox>>(
    submodule, "ComplianceClientBox");
  ExportApplicationComplianceClient(submodule);
  ExportComplianceParameter(submodule);
  ExportComplianceRuleEntry(submodule);
  ExportComplianceRuleSchema(submodule);
  ExportComplianceRuleViolationRecord(submodule);
  register_exception<ComplianceCheckException>(submodule,
    "ComplianceCheckException");
}

void Nexus::Python::ExportComplianceParameter(module& module) {
  class_<ComplianceParameter>(module, "ComplianceParameter").
    def(init()).
    def(init<std::string, ComplianceValue>()).
    def_readwrite("name", &ComplianceParameter::m_name).
    def_readwrite("value", &ComplianceParameter::m_value).
    def(self == self).
    def(self != self);
}

void Nexus::Python::ExportComplianceRuleEntry(module& module) {
  class_<ComplianceRuleEntry>(module, "ComplianceRuleEntry").
    def(init()).
    def(init<ComplianceRuleId, DirectoryEntry, ComplianceRuleEntry::State,
      ComplianceRuleSchema>()).
    def_property("id", &ComplianceRuleEntry::GetId,
      &ComplianceRuleEntry::SetId).
    def_property_readonly("directory_entry",
      &ComplianceRuleEntry::GetDirectoryEntry).
    def_property("state", &ComplianceRuleEntry::GetState,
      &ComplianceRuleEntry::SetState).
    def_property_readonly("schema", &ComplianceRuleEntry::GetSchema).
    def(self == self).
    def(self != self);
}

void Nexus::Python::ExportComplianceRuleSchema(module& module) {
  class_<ComplianceRuleSchema>(module, "ComplianceRuleSchema").
    def(init()).
    def(init<std::string, std::vector<ComplianceParameter>>()).
    def_property_readonly("name", &ComplianceRuleSchema::GetName).
    def_property_readonly("parameters", &ComplianceRuleSchema::GetParameters).
    def(self == self).
    def(self != self);
}

void Nexus::Python::ExportComplianceRuleViolationRecord(module& module) {
  class_<ComplianceRuleViolationRecord>(module,
    "ComplianceRuleViolationRecord").
    def(init()).
    def(init<DirectoryEntry, OrderId, ComplianceRuleId, std::string,
      std::string>()).
    def(init<DirectoryEntry, OrderId, ComplianceRuleId, std::string,
      std::string, ptime>()).
    def_readwrite("account", &ComplianceRuleViolationRecord::m_account).
    def_readwrite("order_id", &ComplianceRuleViolationRecord::m_orderId).
    def_readwrite("rule_id", &ComplianceRuleViolationRecord::m_ruleId).
    def_readwrite("schema_name", &ComplianceRuleViolationRecord::m_schemaName).
    def_readwrite("reason", &ComplianceRuleViolationRecord::m_reason).
    def_readwrite("timestamp", &ComplianceRuleViolationRecord::m_timestamp);
}

void Nexus::Python::ExportComplianceTestEnvironment(module& module) {
  class_<ComplianceTestEnvironment>(module, "ComplianceTestEnvironment").
    def(init<ServiceLocatorClientBox, AdministrationClientBox, TimeClientBox>(),
      call_guard<GilRelease>()).
    def("__del__",
      [] (ComplianceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def("close", &ComplianceTestEnvironment::Close, call_guard<GilRelease>()).
    def("make_client",
      [] (ComplianceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonComplianceClient(self.MakeClient(
          serviceLocatorClient));
      }, call_guard<GilRelease>());
}
