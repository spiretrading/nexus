#include "Nexus/Python/Compliance.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Compliance/CachedComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStoreException.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/ComplianceRuleViolationRecord.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/SqlComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonComplianceClient.hpp"
#include "Nexus/Python/ToPythonComplianceRuleDataStore.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::Python;
using namespace pybind11;

void Nexus::Python::export_cached_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    CachedComplianceRuleDataStore<ComplianceRuleDataStore>>;
  auto data_store = export_compliance_rule_data_store<DataStore>(
    module, "CachedComplianceRuleDataStore");
  data_store.def(init<ComplianceRuleDataStore>());
}

void Nexus::Python::export_compliance(module& module) {
  auto submodule = module.def_submodule("compliance");
  export_cached_compliance_rule_data_store(submodule);
  export_compliance_client<ToPythonComplianceClient<ComplianceClient>>(
    submodule, "ComplianceClient");
  export_compliance_parameter(submodule);
  export_compliance_rule_data_store<
    ToPythonComplianceRuleDataStore<ComplianceRuleDataStore>>(
      submodule, "ComplianceRuleDataStore");
  export_compliance_rule_data_store_exception(submodule);
  export_compliance_rule_entry(submodule);
  export_compliance_rule_schema(submodule);
  export_compliance_rule_violation_record(submodule);
  export_local_compliance_rule_data_store(submodule);
  export_mysql_compliance_rule_data_store(submodule);
  export_sqlite_compliance_rule_data_store(submodule);
  auto tests_submodule = submodule.def_submodule("tests");
  export_compliance_test_environment(tests_submodule);
}

void Nexus::Python::export_compliance_parameter(module& module) {
  class_<ComplianceParameter>(module, "ComplianceParameter").
    def(init()).
    def(init<const ComplianceParameter&>()).
    def(init<std::string, ComplianceValue>(), arg("name"), arg("value")).
    def_readwrite("name", &ComplianceParameter::m_name).
    def_readwrite("value", &ComplianceParameter::m_value).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, ComplianceParameter>);
}

void Nexus::Python::export_compliance_rule_data_store_exception(
    module& module) {
  register_exception<ComplianceRuleDataStoreException>(
    module, "ComplianceRuleDataStoreException", GetIOException());
}

void Nexus::Python::export_compliance_rule_entry(module& module) {
  auto entry = class_<ComplianceRuleEntry>(module, "ComplianceRuleEntry").
    def(init()).
    def(init<const ComplianceRuleEntry&>()).
    def(init<ComplianceRuleEntry::Id, DirectoryEntry,
      ComplianceRuleEntry::State, ComplianceRuleSchema>(), arg("id"),
      arg("directory_entry"), arg("state"), arg("schema")).
    def_property_readonly("id", &ComplianceRuleEntry::get_id).
    def_property_readonly("directory_entry",
      &ComplianceRuleEntry::get_directory_entry).
    def_property("state", &ComplianceRuleEntry::get_state,
      &ComplianceRuleEntry::set_state).
    def_property_readonly("schema", &ComplianceRuleEntry::get_schema).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, ComplianceRuleEntry>);
  enum_<ComplianceRuleEntry::State::Type>(entry, "State")
    .value("ACTIVE", ComplianceRuleEntry::State::ACTIVE)
    .value("PASSIVE", ComplianceRuleEntry::State::PASSIVE)
    .value("DISABLED", ComplianceRuleEntry::State::DISABLED)
    .value("DELETED", ComplianceRuleEntry::State::DELETED);
}

void Nexus::Python::export_compliance_rule_schema(module& module) {
  class_<ComplianceRuleSchema>(module, "ComplianceRuleSchema").
    def(init()).
    def(init<const ComplianceRuleSchema&>()).
    def(init<std::string, std::vector<ComplianceParameter>>(), arg("name"),
      arg("parameters")).
    def_property_readonly("name", &ComplianceRuleSchema::get_name).
    def_property_readonly("parameters", &ComplianceRuleSchema::get_parameters).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, ComplianceRuleSchema>);
}

void Nexus::Python::export_compliance_rule_violation_record(module& module) {
  class_<ComplianceRuleViolationRecord>(
    module, "ComplianceRuleViolationRecord").
      def(init()).
      def(init<const ComplianceRuleViolationRecord&>()).
      def_readwrite("account", &ComplianceRuleViolationRecord::m_account).
      def_readwrite("order_id", &ComplianceRuleViolationRecord::m_order_id).
      def_readwrite("rule_id", &ComplianceRuleViolationRecord::m_rule_id).
      def_readwrite("schema_name",
        &ComplianceRuleViolationRecord::m_schema_name).
      def_readwrite("reason", &ComplianceRuleViolationRecord::m_reason).
      def_readwrite("timestamp", &ComplianceRuleViolationRecord::m_timestamp).
      def(self == self).
      def(self != self).
      def("__str__",
        &boost::lexical_cast<std::string, ComplianceRuleViolationRecord>);
}

void Nexus::Python::export_compliance_test_environment(module& module) {
  class_<ComplianceTestEnvironment>(module, "ComplianceTestEnvironment")
    .def(init<ServiceLocatorClientBox, AdministrationClient, TimeClientBox>(),
      call_guard<GilRelease>(), arg("service_locator_client"),
      arg("administration_client"), arg("time_client"))
    .def("make_client", [] (ComplianceTestEnvironment& self,
          ServiceLocatorClientBox service_locator_client) {
        return ToPythonComplianceClient(
          self.make_client(std::move(service_locator_client)));
      }, call_guard<GilRelease>(), arg("service_locator_client"))
    .def("close", &ComplianceTestEnvironment::close, call_guard<GilRelease>());
}

void Nexus::Python::export_local_compliance_rule_data_store(module& module) {
  using DataStore =
    ToPythonComplianceRuleDataStore<LocalComplianceRuleDataStore>;
  auto data_store = export_compliance_rule_data_store<DataStore>(
    module, "LocalComplianceRuleDataStore");
  data_store.def(init());
}

void Nexus::Python::export_mysql_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    SqlComplianceRuleDataStore<SqlConnection<Viper::MySql::Connection>>>;
  auto data_store = export_compliance_rule_data_store<DataStore>(
    module, "MySqlComplianceRuleDataStore");
  data_store.def(init([] (std::string host, unsigned int port,
      std::string username, std::string password, std::string database) {
    return std::make_shared<DataStore>(
      std::make_unique<SqlConnection<Viper::MySql::Connection>>(
        Viper::MySql::Connection(host, port, username, password, database)));
  }), call_guard<GilRelease>());
}

void Nexus::Python::export_sqlite_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    SqlComplianceRuleDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  auto data_store = export_compliance_rule_data_store<DataStore>(
    module, "SqliteComplianceRuleDataStore");
  data_store.def(init([] (std::string path) {
    return std::make_shared<DataStore>(
      std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(path));
  }), call_guard<GilRelease>());
}
