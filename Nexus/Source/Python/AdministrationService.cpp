#include "Nexus/Python/AdministrationService.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/ApplicationDefinitions.hpp"
#include "Nexus/AdministrationService/CachedAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/SqlAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonAdministrationClient.hpp"
#include "Nexus/Python/ToPythonAdministrationDataStore.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto administration_client = std::unique_ptr<class_<AdministrationClient>>();
  auto administration_data_store =
    std::unique_ptr<class_<AdministrationDataStore>>();
}

class_<AdministrationClient>& Nexus::Python::
    get_exported_administration_client() {
  return *administration_client;
}

class_<AdministrationDataStore>& Nexus::Python::
    get_exported_administration_data_store() {
  return *administration_data_store;
}

void Nexus::Python::export_account_identity(module& module) {
  export_default_methods(class_<AccountIdentity>(module, "AccountIdentity")).
    def_readwrite("registration_time", &AccountIdentity::m_registration_time).
    def_readwrite("last_login_time", &AccountIdentity::m_last_login_time).
    def_readwrite("first_name", &AccountIdentity::m_first_name).
    def_readwrite("last_name", &AccountIdentity::m_last_name).
    def_readwrite("email_address", &AccountIdentity::m_email_address).
    def_readwrite("address_line_one", &AccountIdentity::m_address_line_one).
    def_readwrite("address_line_two", &AccountIdentity::m_address_line_two).
    def_readwrite("address_line_three", &AccountIdentity::m_address_line_three).
    def_readwrite("city", &AccountIdentity::m_city).
    def_readwrite("province", &AccountIdentity::m_province).
    def_readwrite("country", &AccountIdentity::m_country).
    def_readwrite("photo_id", &AccountIdentity::m_photo_id).
    def_readwrite("user_notes", &AccountIdentity::m_user_notes);
}

void Nexus::Python::export_account_modification_request(module& module) {
  auto request =
    export_default_methods(
      class_<AccountModificationRequest>(module, "AccountModificationRequest")).
        def(init<AccountModificationRequest::Id,
          AccountModificationRequest::Type, DirectoryEntry, DirectoryEntry,
          ptime>()).
        def_property_readonly("id", &AccountModificationRequest::get_id).
        def_property_readonly("type", &AccountModificationRequest::get_type).
        def_property_readonly(
          "account", &AccountModificationRequest::get_account).
        def_property_readonly("submission_account",
          &AccountModificationRequest::get_submission_account).
        def_property_readonly("timestamp",
          &AccountModificationRequest::get_timestamp);
  enum_<AccountModificationRequest::Type>(request, "Type").
    value("ENTITLEMENTS", AccountModificationRequest::Type::ENTITLEMENTS).
    value("RISK", AccountModificationRequest::Type::RISK);
  enum_<AccountModificationRequest::Status>(request, "Status").
    value("NONE", AccountModificationRequest::Status::NONE).
    value("PENDING", AccountModificationRequest::Status::PENDING).
    value("REVIEWED", AccountModificationRequest::Status::REVIEWED).
    value("SCHEDULED", AccountModificationRequest::Status::SCHEDULED).
    value("GRANTED", AccountModificationRequest::Status::GRANTED).
    value("REJECTED", AccountModificationRequest::Status::REJECTED);
  export_default_methods(class_<AccountModificationRequest::Update>(
      request, "Update")).
    def(init<AccountModificationRequest::Status, DirectoryEntry, int, ptime>()).
    def_readwrite("status", &AccountModificationRequest::Update::m_status).
    def_readwrite("account", &AccountModificationRequest::Update::m_account).
    def_readwrite("sequence_number",
      &AccountModificationRequest::Update::m_sequence_number).
    def_readwrite("timestamp",
      &AccountModificationRequest::Update::m_timestamp);
  module.def("is_terminal", [] (AccountModificationRequest::Status status) {
    return is_terminal(status);
  });
}

void Nexus::Python::export_account_roles(module& module) {
  enum_<AccountRole::Type>(module, "AccountRole").
    value("TRADER", AccountRole::TRADER).
    value("MANAGER", AccountRole::MANAGER).
    value("SERVICE", AccountRole::SERVICE).
    value("ADMINISTRATOR", AccountRole::ADMINISTRATOR);
  export_enum_set<AccountRoles>(module, "AccountRoles");
}

void Nexus::Python::export_administration_data_store_exception(module& module) {
  register_exception<AdministrationDataStoreException>(
    module, "AdministrationDataStoreException", get_io_exception());
}

void Nexus::Python::export_administration_service(module& module) {
  administration_client = std::make_unique<class_<AdministrationClient>>(
    export_administration_client<AdministrationClient>(
      module, "AdministrationClient"));
  administration_data_store = std::make_unique<class_<AdministrationDataStore>>(
    export_administration_data_store<AdministrationDataStore>(
      module, "AdministrationDataStore"));
  export_administration_service_application_definitions(module);
  export_account_identity(module);
  export_account_modification_request(module);
  export_account_roles(module);
  module.def("load_risk_parameters",
    [] (AdministrationClient& client, const DirectoryEntry& account) {
      return load_risk_parameters(client, account);
    }, call_guard<gil_scoped_release>());
  export_administration_data_store_exception(module);
  export_cached_administration_data_store(module);
  export_entitlement_modification(module);
  export_indexed_account_identity(module);
  export_indexed_risk_parameters(module);
  export_indexed_risk_state(module);
  export_local_administration_data_store(module);
  export_message(module);
  export_mysql_administration_data_store(module);
  export_risk_modification(module);
  export_sqlite_administration_data_store(module);
  export_trading_group(module);
  export_queue_suite<RiskState>(module, "RiskState");
  export_queue_suite<RiskParameters>(module, "RiskParameters");
  auto test_module = module.def_submodule("tests");
  export_administration_service_test_environment(test_module);
}

void Nexus::Python::export_administration_service_application_definitions(
    module& module) {
  export_administration_client<
    ToPythonAdministrationClient<ApplicationAdministrationClient>>(
      module, "ApplicationAdministrationClient").
    def(pybind11::init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>&
          client) {
        return std::make_unique<ToPythonAdministrationClient<
          ApplicationAdministrationClient>>(Ref(client.get()));
      }), keep_alive<1, 2>());
}

void Nexus::Python::export_administration_service_test_environment(
    module& module) {
  using TestEnvironment = AdministrationServiceTestEnvironment;
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>>(
      module, "AdministrationServiceTestEnvironment").
    def(pybind11::init(
      &make_python_shared<TestEnvironment, ServiceLocatorClient&>),
      keep_alive<1, 2>()).
    def(pybind11::init(&make_python_shared<TestEnvironment,
      ServiceLocatorClient&, const EntitlementDatabase&>), keep_alive<1, 2>()).
    def_property_readonly("client", [] (TestEnvironment& self) {
      return ToPythonAdministrationClient(self.get_client());
    }).
    def("make_administrator", &TestEnvironment::make_administrator,
      call_guard<gil_scoped_release>()).
    def("make_client",
      [] (TestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonAdministrationClient(self.make_client(Ref(client)));
      }, call_guard<gil_scoped_release>(), keep_alive<0, 2>()).
    def("close", &TestEnvironment::close, call_guard<gil_scoped_release>());
  module.def("make_administrator_account",
    &make_administrator_account<ServiceLocatorClient>,
    call_guard<gil_scoped_release>());
  module.def("make_administration_service_test_environment",
    &make_administration_service_test_environment,
    call_guard<gil_scoped_release>());
  module.def("grant_all_entitlements", &grant_all_entitlements);
}

void Nexus::Python::export_cached_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    CachedAdministrationDataStore<AdministrationDataStore>>;
  export_administration_data_store<DataStore>(
      module, "CachedAdministrationDataStore").
    def(init<AdministrationDataStore>());
}

void Nexus::Python::export_entitlement_modification(module& module) {
  export_default_methods(class_<EntitlementModification>(
      module, "EntitlementModification")).
    def(init<const std::vector<DirectoryEntry>&>()).
    def_property_readonly(
      "entitlements", &EntitlementModification::get_entitlements);
}

void Nexus::Python::export_indexed_account_identity(module& module) {
  export_default_methods(
    class_<AdministrationDataStore::IndexedAccountIdentity>(
      module, "IndexedAccountIdentity")).
    def_readwrite("index",
      &AdministrationDataStore::IndexedAccountIdentity::m_index).
    def_readwrite("identity",
      &AdministrationDataStore::IndexedAccountIdentity::m_identity);
}

void Nexus::Python::export_indexed_risk_parameters(module& module) {
  export_default_methods(class_<AdministrationDataStore::IndexedRiskParameters>(
      module, "IndexedRiskParameters")).
    def_readwrite("index",
      &AdministrationDataStore::IndexedRiskParameters::m_index).
    def_readwrite("parameters",
      &AdministrationDataStore::IndexedRiskParameters::m_parameters);
}

void Nexus::Python::export_indexed_risk_state(module& module) {
  export_default_methods(class_<AdministrationDataStore::IndexedRiskState>(
      module, "IndexedRiskState")).
    def_readwrite("index", &AdministrationDataStore::IndexedRiskState::m_index).
    def_readwrite("state", &AdministrationDataStore::IndexedRiskState::m_state);
}

void Nexus::Python::export_local_administration_data_store(module& module) {
  using DataStore =
    ToPythonAdministrationDataStore<LocalAdministrationDataStore>;
  export_administration_data_store<DataStore>(
      module, "LocalAdministrationDataStore").
    def(init());
}

void Nexus::Python::export_message(module& module) {
  auto message = export_default_methods(class_<Message>(module, "Message")).
    def(init<
      Message::Id, DirectoryEntry, ptime, const std::vector<Message::Body>&>()).
    def_property_readonly("id", &Message::get_id).
    def_property_readonly("account", &Message::get_account).
    def_property_readonly("timestamp", &Message::get_timestamp).
    def_property_readonly("body", &Message::get_body).
    def_property_readonly("bodies", &Message::get_bodies);
  export_default_methods(class_<Message::Body>(message, "Body")).
    def_readwrite("content_type", &Message::Body::m_content_type).
    def_readwrite("message", &Message::Body::m_message).
    def_readonly_static("NONE", &Message::Body::EMPTY).
    def_static("make_plain_text", &Message::Body::make_plain_text);
}

void Nexus::Python::export_mysql_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_administration_data_store<DataStore>(
      module, "MySqlAdministrationDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database,
        const SqlAdministrationDataStore<SqlConnection<
          Viper::MySql::Connection>>::DirectoryEntrySource& source) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::MySql::Connection>>(
          Viper::MySql::Connection(host, port, username, password, database)),
          source);
    }), call_guard<gil_scoped_release>());
}

void Nexus::Python::export_risk_modification(module& module) {
  export_default_methods(class_<RiskModification>(module, "RiskModification")).
    def(init<RiskParameters>()).
    def_property_readonly("parameters", &RiskModification::get_parameters);
}

void Nexus::Python::export_sqlite_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  export_administration_data_store<DataStore>(
      module, "SqliteAdministrationDataStore").
    def(init([] (std::string path, const SqlAdministrationDataStore<
      SqlConnection<Viper::Sqlite3::Connection>>::DirectoryEntrySource&
        source) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(path),
        source);
    }), call_guard<gil_scoped_release>());
}

void Nexus::Python::export_trading_group(module& module) {
  export_default_methods(class_<TradingGroup>(module, "TradingGroup")).
    def(init<DirectoryEntry, DirectoryEntry, std::vector<DirectoryEntry>,
      DirectoryEntry, std::vector<DirectoryEntry>>()).
    def_property_readonly("entry", &TradingGroup::get_entry).
    def_property_readonly(
      "managers_directory", &TradingGroup::get_managers_directory).
    def_property_readonly("managers", &TradingGroup::get_managers).
    def_property_readonly(
      "traders_directory", &TradingGroup::get_traders_directory).
    def_property_readonly("traders", &TradingGroup::get_traders);
}
