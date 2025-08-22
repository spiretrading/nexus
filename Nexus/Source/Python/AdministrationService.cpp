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
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace Nexus::RiskService;
using namespace Nexus::Tests;
using namespace pybind11;

void Nexus::Python::export_account_identity(module& module) {
  class_<AccountIdentity>(module, "AccountIdentity").
    def(init()).
    def(init<const AccountIdentity&>()).
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
    def_readwrite("user_notes", &AccountIdentity::m_user_notes).
    def("__str__", &boost::lexical_cast<std::string, AccountIdentity>);
}

void Nexus::Python::export_account_modification_request(module& module) {
  auto request =
    class_<AccountModificationRequest>(module, "AccountModificationRequest").
      def(init<const AccountModificationRequest&>()).
      def(init<AccountModificationRequest::Id, AccountModificationRequest::Type,
        DirectoryEntry, DirectoryEntry, ptime>()).
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
  class_<AccountModificationRequest::Update>(request, "Update").
    def(init()).
    def(init<const AccountModificationRequest::Update&>()).
    def(init<AccountModificationRequest::Status, DirectoryEntry, int, ptime>()).
    def_readwrite("status", &AccountModificationRequest::Update::m_status).
    def_readwrite("account", &AccountModificationRequest::Update::m_account).
    def_readwrite("sequence_number",
      &AccountModificationRequest::Update::m_sequence_number).
    def_readwrite("timestamp",
      &AccountModificationRequest::Update::m_timestamp).
    def(self == self).
    def(self != self).
    def("__str__",
      &boost::lexical_cast<std::string, AccountModificationRequest::Update>);
  module.def("is_terminal", &is_terminal);
}

void Nexus::Python::export_account_roles(module& module) {
  enum_<AccountRole::Type>(module, "AccountRole").
    value("TRADER", AccountRole::TRADER).
    value("MANAGER", AccountRole::MANAGER).
    value("SERVICE", AccountRole::SERVICE).
    value("ADMINISTRATOR", AccountRole::ADMINISTRATOR);
  ExportEnumSet<AccountRoles>(module, "AccountRoles");
}

void Nexus::Python::export_administration_data_store_exception(module& module) {
  register_exception<AdministrationDataStoreException>(
    module, "AdministrationDataStoreException", GetIOException());
}

void Nexus::Python::export_administration_service(module& module) {
  auto submodule = module.def_submodule("administration_service");
  export_account_identity(submodule);
  export_account_modification_request(submodule);
  export_account_roles(submodule);
  export_administration_client<ToPythonAdministrationClient<
    AdministrationClient>>(submodule, "AdministrationClient");
  submodule.def("load_risk_parameters",
    [] (AdministrationClient& client, const DirectoryEntry& account) {
      return load_risk_parameters(client, account);
    }, call_guard<GilRelease>());
  export_administration_data_store<ToPythonAdministrationDataStore<
    AdministrationDataStore>>(submodule, "AdministrationDataStore");
  export_administration_data_store_exception(submodule);
  export_cached_administration_data_store(submodule);
  export_entitlement_modification(submodule);
  export_indexed_account_identity(submodule);
  export_indexed_risk_parameters(submodule);
  export_indexed_risk_state(submodule);
  export_local_administration_data_store(submodule);
  export_message(submodule);
  export_mysql_administration_data_store(submodule);
  export_risk_modification(submodule);
  export_sqlite_administration_data_store(submodule);
  export_trading_group(submodule);
  ExportQueueSuite<RiskState>(submodule, "RiskState");
  ExportQueueSuite<RiskParameters>(submodule, "RiskParameters");
  auto test_module = submodule.def_submodule("tests");
  export_administration_service_test_environment(test_module);
}

void Nexus::Python::export_administration_service_test_environment(
    module& module) {
  using TestEnvironment = AdministrationServiceTestEnvironment;
  class_<TestEnvironment>(module, "AdministrationServiceTestEnvironment").
    def(init<ServiceLocatorClientBox>(), call_guard<GilRelease>()).
    def(init<ServiceLocatorClientBox, EntitlementDatabase>(),
      call_guard<GilRelease>()).
    def("__del__", [] (AdministrationServiceTestEnvironment& self) {
      self.close();
    }, call_guard<GilRelease>()).
    def_property_readonly("client", [] (TestEnvironment& self) {
      return ToPythonAdministrationClient(self.get_client());
    }).
    def("make_administrator", &TestEnvironment::make_administrator,
      call_guard<GilRelease>()).
    def("make_client", [] (TestEnvironment& self,
        ServiceLocatorClientBox service_locator_client) {
      return ToPythonAdministrationClient(
        self.make_client(std::move(service_locator_client)));
    }, call_guard<GilRelease>()).
    def("close", &TestEnvironment::close, call_guard<GilRelease>());
  module.def("make_administrator_account",
    &make_administrator_account<ServiceLocatorClientBox>,
    call_guard<GilRelease>());
  module.def("make_administration_service_test_environment",
    &make_administration_service_test_environment, call_guard<GilRelease>());
  module.def("grant_all_entitlements", &grant_all_entitlements);
}

void Nexus::Python::export_cached_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    CachedAdministrationDataStore<AdministrationDataStore>>;
  auto data_store = export_administration_data_store<DataStore>(
    module, "CachedAdministrationDataStore");
  data_store.def(init<AdministrationDataStore>());
}

void Nexus::Python::export_entitlement_modification(module& module) {
  class_<EntitlementModification>(module, "EntitlementModification").
    def(init()).
    def(init<const EntitlementModification&>()).
    def(init<const std::vector<DirectoryEntry>&>()).
    def_property_readonly(
      "entitlements", &EntitlementModification::get_entitlements);
}

void Nexus::Python::export_indexed_account_identity(module& module) {
  class_<AdministrationDataStore::IndexedAccountIdentity>(
    module, "IndexedAccountIdentity").
      def(init()).
      def(init<const AdministrationDataStore::IndexedAccountIdentity&>()).
      def_readwrite("index",
        &AdministrationDataStore::IndexedAccountIdentity::m_index).
      def_readwrite("identity",
        &AdministrationDataStore::IndexedAccountIdentity::m_identity);
}

void Nexus::Python::export_indexed_risk_parameters(module& module) {
  class_<AdministrationDataStore::IndexedRiskParameters>(
    module, "IndexedRiskParameters").
      def(init()).
      def(init<const AdministrationDataStore::IndexedRiskParameters&>()).
      def_readwrite("index",
        &AdministrationDataStore::IndexedRiskParameters::m_index).
      def_readwrite("parameters",
        &AdministrationDataStore::IndexedRiskParameters::m_parameters);
}

void Nexus::Python::export_indexed_risk_state(module& module) {
  class_<AdministrationDataStore::IndexedRiskState>(module, "IndexedRiskState").
    def(init()).
    def(init<const AdministrationDataStore::IndexedRiskState&>()).
    def_readwrite("index", &AdministrationDataStore::IndexedRiskState::m_index).
    def_readwrite("state", &AdministrationDataStore::IndexedRiskState::m_state);
}

void Nexus::Python::export_local_administration_data_store(module& module) {
  using DataStore =
    ToPythonAdministrationDataStore<LocalAdministrationDataStore>;
  auto data_store = export_administration_data_store<DataStore>(
    module, "LocalAdministrationDataStore");
  data_store.def(init());
}

void Nexus::Python::export_message(module& module) {
  auto message = class_<Message>(module, "Message").
    def(init()).
    def(init<const Message&>()).
    def(init<
      Message::Id, DirectoryEntry, ptime, const std::vector<Message::Body>&>()).
    def_property_readonly("id", &Message::get_id).
    def_property_readonly("account", &Message::get_account).
    def_property_readonly("timestamp", &Message::get_timestamp).
    def_property_readonly("body", &Message::get_body).
    def_property_readonly("bodies", &Message::get_bodies).
    def(self == self).
    def(self != self);
  class_<Message::Body>(message, "Body").
    def(init()).
    def(init<const Message::Body&>()).
    def_readwrite("content_type", &Message::Body::m_content_type).
    def_readwrite("message", &Message::Body::m_message).
    def_readonly_static("NONE", &Message::Body::EMPTY).
    def_static("make_plain_text", &Message::Body::make_plain_text).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, Message::Body>);
}

void Nexus::Python::export_mysql_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<Viper::MySql::Connection>>>;
  auto data_store = export_administration_data_store<DataStore>(
    module, "MySqlAdministrationDataStore");
  data_store.def(init([] (std::string host, unsigned int port,
      std::string username, std::string password, std::string database,
      const SqlAdministrationDataStore<SqlConnection<
        Viper::MySql::Connection>>::DirectoryEntrySourceFunction& source) {
    return std::make_shared<DataStore>(
      std::make_unique<SqlConnection<Viper::MySql::Connection>>(
        Viper::MySql::Connection(host, port, username, password, database)),
        source);
  }), call_guard<GilRelease>());
}

void Nexus::Python::export_risk_modification(module& module) {
  class_<RiskModification>(module, "RiskModification").
    def(init()).
    def(init<const RiskModification&>()).
    def(init<RiskParameters>()).
    def_property_readonly("parameters", &RiskModification::get_parameters);
}

void Nexus::Python::export_sqlite_administration_data_store(module& module) {
  using DataStore = ToPythonAdministrationDataStore<
    SqlAdministrationDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  auto data_store = export_administration_data_store<DataStore>(
    module, "SqliteAdministrationDataStore");
  data_store.def(init([] (std::string path, const SqlAdministrationDataStore<
      SqlConnection<Viper::Sqlite3::Connection>>::DirectoryEntrySourceFunction&
        source) {
    return std::make_shared<DataStore>(
      std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(path),
      source);
  }), call_guard<GilRelease>());
}

void Nexus::Python::export_trading_group(module& module) {
  class_<TradingGroup>(module, "TradingGroup").
    def(init()).
    def(init<const TradingGroup&>()).
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
