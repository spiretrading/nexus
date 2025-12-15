#include "Nexus/Python/Compliance.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Compliance/ApplicationDefinitions.hpp"
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
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto compliance_client = std::unique_ptr<class_<ComplianceClient>>();
  auto compliance_rule_data_store =
    std::unique_ptr<class_<ComplianceRuleDataStore>>();
}

class_<ComplianceClient>& Nexus::Python::get_exported_compliance_client() {
  return *compliance_client;
}

class_<ComplianceRuleDataStore>& Nexus::Python::
    get_exported_compliance_rule_data_store() {
  return *compliance_rule_data_store;
}

void Nexus::Python::export_cached_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    CachedComplianceRuleDataStore<ComplianceRuleDataStore>>;
  export_compliance_rule_data_store<DataStore>(
      module, "CachedComplianceRuleDataStore").
    def(init<ComplianceRuleDataStore>());
}

void Nexus::Python::export_compliance(module& module) {
  compliance_client = std::make_unique<class_<ComplianceClient>>(
    export_compliance_client<ComplianceClient>(module, "ComplianceClient"));
  compliance_rule_data_store =
    std::make_unique<class_<ComplianceRuleDataStore>>(
      export_compliance_rule_data_store<ComplianceRuleDataStore>(
        module, "ComplianceRuleDataStore"));
  export_compliance_application_definitions(module);
  export_cached_compliance_rule_data_store(module);
  export_compliance_parameter(module);
  export_compliance_rule_data_store_exception(module);
  export_compliance_rule_entry(module);
  export_compliance_rule_schema(module);
  export_compliance_rule_violation_record(module);
  export_local_compliance_rule_data_store(module);
  export_mysql_compliance_rule_data_store(module);
  export_sqlite_compliance_rule_data_store(module);
  auto tests_submodule = module.def_submodule("tests");
  export_compliance_test_environment(tests_submodule);
}

void Nexus::Python::export_compliance_application_definitions(module& module) {
  export_compliance_client<
    ToPythonComplianceClient<ApplicationComplianceClient>>(
      module, "ApplicationComplianceClient").
    def(pybind11::init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>&
          client) {
        return std::make_unique<ToPythonComplianceClient<
          ApplicationComplianceClient>>(Ref(client.get()));
      }), keep_alive<1, 2>());
}

void Nexus::Python::export_compliance_parameter(module& module) {
  export_default_methods(
    class_<ComplianceParameter>(module, "ComplianceParameter")).
    def(init<std::string, ComplianceValue>(), arg("name"), arg("value")).
    def_readwrite("name", &ComplianceParameter::m_name).
    def_readwrite("value", &ComplianceParameter::m_value);
}

void Nexus::Python::export_compliance_rule_data_store_exception(
    module& module) {
  register_exception<ComplianceRuleDataStoreException>(
    module, "ComplianceRuleDataStoreException", get_io_exception());
}

void Nexus::Python::export_compliance_rule_entry(module& module) {
  auto entry = export_default_methods(
    class_<ComplianceRuleEntry>(module, "ComplianceRuleEntry")).
    def(init<ComplianceRuleEntry::Id, DirectoryEntry,
      ComplianceRuleEntry::State, ComplianceRuleSchema>(), arg("id"),
      arg("directory_entry"), arg("state"), arg("schema")).
    def_property_readonly("id", &ComplianceRuleEntry::get_id).
    def_property_readonly("directory_entry",
      &ComplianceRuleEntry::get_directory_entry).
    def_property("state", &ComplianceRuleEntry::get_state,
      &ComplianceRuleEntry::set_state).
    def_property_readonly("schema", &ComplianceRuleEntry::get_schema);
  enum_<ComplianceRuleEntry::State::Type>(entry, "State").
    value("ACTIVE", ComplianceRuleEntry::State::ACTIVE).
    value("PASSIVE", ComplianceRuleEntry::State::PASSIVE).
    value("DISABLED", ComplianceRuleEntry::State::DISABLED).
    value("DELETED", ComplianceRuleEntry::State::DELETED);
}

void Nexus::Python::export_compliance_rule_schema(module& module) {
  export_default_methods(
    class_<ComplianceRuleSchema>(module, "ComplianceRuleSchema")).
    def(init<std::string, std::vector<ComplianceParameter>>(), arg("name"),
      arg("parameters")).
    def_property_readonly("name", &ComplianceRuleSchema::get_name).
    def_property_readonly("parameters", &ComplianceRuleSchema::get_parameters);
}

void Nexus::Python::export_compliance_rule_violation_record(module& module) {
  export_default_methods(class_<ComplianceRuleViolationRecord>(
    module, "ComplianceRuleViolationRecord")).
      def_readwrite("account", &ComplianceRuleViolationRecord::m_account).
      def_readwrite("order_id", &ComplianceRuleViolationRecord::m_order_id).
      def_readwrite("rule_id", &ComplianceRuleViolationRecord::m_rule_id).
      def_readwrite("schema_name",
        &ComplianceRuleViolationRecord::m_schema_name).
      def_readwrite("reason", &ComplianceRuleViolationRecord::m_reason).
      def_readwrite("timestamp", &ComplianceRuleViolationRecord::m_timestamp);
}

void Nexus::Python::export_compliance_test_environment(module& module) {
  class_<ComplianceTestEnvironment, std::shared_ptr<ComplianceTestEnvironment>>(
      module, "ComplianceTestEnvironment")
    .def(init(&make_python_shared<ComplianceTestEnvironment,
      ServiceLocatorClient&, AdministrationClient&, TimeClient&>),
      keep_alive<1, 2>(), keep_alive<1, 3>()).
    def("make_client",
      [] (ComplianceTestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonComplianceClient(self.make_client(Ref(client)));
      }, call_guard<GilRelease>(), keep_alive<0, 2>()).
    def("close", &ComplianceTestEnvironment::close, call_guard<GilRelease>());
}

void Nexus::Python::export_local_compliance_rule_data_store(module& module) {
  using DataStore =
    ToPythonComplianceRuleDataStore<LocalComplianceRuleDataStore>;
  export_compliance_rule_data_store<DataStore>(
    module, "LocalComplianceRuleDataStore").
    def(init());
}

void Nexus::Python::export_mysql_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    SqlComplianceRuleDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_compliance_rule_data_store<DataStore>(
    module, "MySqlComplianceRuleDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::MySql::Connection>>(
          Viper::MySql::Connection(host, port, username, password, database)));
    }), call_guard<GilRelease>());
}

void Nexus::Python::export_sqlite_compliance_rule_data_store(module& module) {
  using DataStore = ToPythonComplianceRuleDataStore<
    SqlComplianceRuleDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  auto data_store = export_compliance_rule_data_store<DataStore>(
    module, "SqliteComplianceRuleDataStore").
    def(init([] (std::string path) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(path));
    }),   call_guard<GilRelease>());
}
