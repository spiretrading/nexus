#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include <pybind11/operators.h>
#include "Nexus/Python/ToPythonRiskClient.hpp"
#include "Nexus/Python/ToPythonRiskDataStore.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskState.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto risk_client = std::unique_ptr<class_<RiskClient>>();
  auto risk_data_store = std::unique_ptr<class_<RiskDataStore>>();
}

class_<RiskClient>& Nexus::Python::get_exported_risk_client() {
  return *risk_client;
}

class_<RiskDataStore>& Nexus::Python::get_exported_risk_data_store() {
  return *risk_data_store;
}

void Nexus::Python::export_inventory_snapshot(module& module) {
  export_default_methods(
      class_<InventorySnapshot>(module, "InventorySnapshot")).
    def(init<const std::vector<Inventory>&, Beam::Sequence,
      const std::vector<OrderId>&>()).
    def_readwrite("inventories", &InventorySnapshot::m_inventories).
    def_readwrite("sequence", &InventorySnapshot::m_sequence).
    def_readwrite("excluded_orders", &InventorySnapshot::m_excluded_orders);
  module.def("strip", &strip);
  module.def("make_portfolio",
    [] (const InventorySnapshot& snapshot, const DirectoryEntry& account,
        const VenueDatabase& venues, OrderExecutionClient& client) {
      return make_portfolio(snapshot, account, venues, client);
    }, keep_alive<0, 4>(), call_guard<GilRelease>());
}

void Nexus::Python::export_local_risk_data_store(module& module) {
  export_risk_data_store<LocalRiskDataStore>(module, "LocalRiskDataStore").
    def(init());
}

void Nexus::Python::export_mysql_risk_data_store(module& module) {
  using DataStore = SqlRiskDataStore<SqlConnection<Viper::MySql::Connection>>;
  class_<ToPythonRiskDataStore<DataStore>>(module, "MySqlRiskDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_unique<ToPythonRiskDataStore<DataStore>>(
        std::make_unique<SqlConnection<Viper::MySql::Connection>>(
          Viper::MySql::Connection(std::move(host), port, std::move(username),
            std::move(password), std::move(database))));
    }));
}

void Nexus::Python::export_risk_parameters(module& module) {
  export_default_methods(class_<RiskParameters>(module, "RiskParameters")).
    def(init<CurrencyId, Money, RiskState, Money, time_duration>()).
    def_readwrite("currency", &RiskParameters::m_currency).
    def_readwrite("buying_power", &RiskParameters::m_buying_power).
    def_readwrite("allowed_state", &RiskParameters::m_allowed_state).
    def_readwrite("net_loss", &RiskParameters::m_net_loss).
    def_readwrite("transition_time", &RiskParameters::m_transition_time);
}

void Nexus::Python::export_risk_service(module& module) {
  risk_client = std::make_unique<class_<RiskClient>>(
    export_risk_client<RiskClient>(module, "RiskClient"));
  risk_data_store = std::make_unique<class_<RiskDataStore>>(
    export_risk_data_store<RiskDataStore>(module, "RiskDataStore"));
  export_inventory_snapshot(module);
  export_local_risk_data_store(module);
  export_mysql_risk_data_store(module);
  export_risk_parameters(module);
  export_risk_service_application_definitions(module);
  export_risk_state(module);
  export_sqlite_risk_data_store(module);
  export_queue_suite<KeyValuePair<RiskPortfolioKey, Inventory>>(
    module, "RiskPortfolioUpdateEntry");
  auto tests_submodule = module.def_submodule("tests");
  export_risk_service_test_environment(tests_submodule);
}

void Nexus::Python::export_risk_service_application_definitions(
    module& module) {
  export_risk_client<ToPythonRiskClient<ApplicationRiskClient>>(
      module, "ApplicationRiskClient").
    def(pybind11::init([] (
        ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>& client) {
      return std::make_unique<ToPythonRiskClient<ApplicationRiskClient>>(
        Ref(client.get()));
    }), keep_alive<1, 2>());
}

void Nexus::Python::export_risk_service_test_environment(module& module) {
  class_<RiskServiceTestEnvironment,
      std::shared_ptr<RiskServiceTestEnvironment>>(
        module, "RiskServiceTestEnvironment").
    def(init([] (ServiceLocatorClient& service_locator_client,
          AdministrationClient& administration_client,
          MarketDataClient& market_data_client,
          OrderExecutionClient& order_execution_client,
          std::function<std::shared_ptr<Timer> ()> timer_builder,
          TimeClient& time_client, const ExchangeRateTable& exchange_rates,
          const VenueDatabase& venue_database,
          const DestinationDatabase& destination_database) {
        auto timer_adaptor = [timer_builder = std::move(timer_builder)] {
          return std::make_unique<Timer>(timer_builder());
        };
        return make_python_shared<RiskServiceTestEnvironment>(
          service_locator_client, administration_client, market_data_client,
          order_execution_client, std::move(timer_adaptor), time_client,
          exchange_rates, venue_database, destination_database);
      }), keep_alive<1, 2>(), keep_alive<1, 3>(),
      keep_alive<1, 4>(), keep_alive<1, 5>(), keep_alive<1, 7>()).
    def("make_client",
      [] (RiskServiceTestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonRiskClient(self.make_client(Ref(client)));
      }, call_guard<GilRelease>(), keep_alive<0, 2>()).
    def("close", &RiskServiceTestEnvironment::close, call_guard<GilRelease>());
}

void Nexus::Python::export_risk_state(module& module) {
  auto risk_state =
      export_default_methods(class_<RiskState>(module, "RiskState")).
    def(init<RiskState::Type>()).
    def(init<RiskState::Type, boost::posix_time::ptime>()).
    def_readwrite("type", &RiskState::m_type).
    def_readwrite("expiry", &RiskState::m_expiry);
  enum_<RiskState::Type::Type>(risk_state, "Type").
    value("NONE", RiskState::Type::NONE).
    value("ACTIVE", RiskState::Type::ACTIVE).
    value("CLOSE_ORDERS", RiskState::Type::CLOSE_ORDERS).
    value("DISABLED", RiskState::Type::DISABLED);
}

void Nexus::Python::export_sqlite_risk_data_store(module& module) {
  using DataStore = SqlRiskDataStore<SqlConnection<Viper::Sqlite3::Connection>>;
  class_<ToPythonRiskDataStore<DataStore>>(module, "SqliteRiskDataStore").
    def(init([] (std::string path) {
      return std::make_unique<ToPythonRiskDataStore<DataStore>>(
        std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(
          Viper::Sqlite3::Connection(std::move(path))));
    }));
}
