#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/lexical_cast.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include <pybind11/operators.h>
#include "Nexus/Python/ToPythonRiskClient.hpp"
#include "Nexus/Python/ToPythonRiskDataStore.hpp"
#include "Nexus/RiskService/InventorySnapshot.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/RiskState.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Nexus::RiskService::Tests;
using namespace pybind11;

void Nexus::Python::export_inventory_snapshot(module& module) {
  class_<InventorySnapshot>(module, "InventorySnapshot").
    def(init()).
    def(init<
      const std::vector<Inventory>&, Sequence, const std::vector<OrderId>&>()).
    def(init<const InventorySnapshot&>()).
    def_readwrite("inventories", &InventorySnapshot::m_inventories).
    def_readwrite("sequence", &InventorySnapshot::m_sequence).
    def_readwrite("excluded_orders", &InventorySnapshot::m_excluded_orders).
    def(self == self).
    def(self != self);
  module.def("strip", &strip);
  module.def("make_portfolio",
    [] (const InventorySnapshot& snapshot, const DirectoryEntry& account,
        const VenueDatabase& venues, OrderExecutionClient& client) {
      return make_portfolio(snapshot, account, venues, client);
    });
}

void Nexus::Python::export_local_risk_data_store(module& module) {
  auto data_store =
    export_risk_data_store<LocalRiskDataStore>(module, "LocalRiskDataStore");
  data_store.def(init());
}

void Nexus::Python::export_mysql_risk_data_store(module& module) {
  using DataStore = SqlRiskDataStore<SqlConnection<Viper::MySql::Connection>>;
  class_<ToPythonRiskDataStore<DataStore>>(module, "MySqlRiskDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_unique<ToPythonRiskDataStore<DataStore>>(
        MakeSqlConnection(Viper::MySql::Connection(
          std::move(host), port, std::move(username), std::move(password),
          std::move(database))));
    }));
}

void Nexus::Python::export_risk_parameters(module& module) {
  class_<RiskParameters>(module, "RiskParameters").
    def(init()).
    def(init<const RiskParameters&>()).
    def(init<CurrencyId, Money, RiskState, Money,
      boost::posix_time::time_duration>()).
    def_readwrite("currency", &RiskParameters::m_currency).
    def_readwrite("buying_power", &RiskParameters::m_buying_power).
    def_readwrite("allowed_state", &RiskParameters::m_allowed_state).
    def_readwrite("net_loss", &RiskParameters::m_net_loss).
    def_readwrite("transition_time", &RiskParameters::m_transition_time).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, RiskParameters>);
}

void Nexus::Python::export_risk_service(module& module) {
  auto submodule = module.def_submodule("risk_service");
  export_inventory_snapshot(submodule);
  export_local_risk_data_store(submodule);
  export_mysql_risk_data_store(submodule);
  export_risk_client<ToPythonRiskClient<RiskClient>>(submodule, "RiskClient");
  export_risk_data_store<ToPythonRiskDataStore<RiskDataStore>>(
    submodule, "RiskDataStore");
  export_risk_parameters(submodule);
  export_risk_state(submodule);
  export_sqlite_risk_data_store(submodule);
  ExportQueueSuite<KeyValuePair<RiskPortfolioKey, Inventory>>(
    module, "RiskPortfolioUpdateEntry");
  auto tests_submodule = submodule.def_submodule("tests");
  export_risk_service_test_environment(tests_submodule);
}

void Nexus::Python::export_risk_service_test_environment(module& module) {
  class_<RiskServiceTestEnvironment>(module, "RiskServiceTestEnvironment").
    def(init([] (ServiceLocatorClientBox service_locator_client,
        AdministrationClient administration_client,
        MarketDataClient market_data_client,
        OrderExecutionClient order_execution_client,
        std::function<std::shared_ptr<TimerBox> ()> timer_factory,
        TimeClientBox time_client, const ExchangeRateTable& exchange_rates,
        const VenueDatabase& venues, const DestinationDatabase& destinations) {
      auto adapted_timer_factory = [=] {
        return std::make_unique<TimerBox>(timer_factory());
      };
      return std::make_unique<RiskServiceTestEnvironment>(
        std::move(service_locator_client), std::move(administration_client),
        std::move(market_data_client), std::move(order_execution_client),
        std::move(adapted_timer_factory), std::move(time_client),
        std::move(exchange_rates), std::move(venues), std::move(destinations));
    }), call_guard<GilRelease>()).
    def("__del__", [] (RiskServiceTestEnvironment& self) {
      self.close();
    }, call_guard<GilRelease>()).
    def("make_client", [] (RiskServiceTestEnvironment& self,
        ServiceLocatorClientBox service_locator_client) {
      return ToPythonRiskClient(
        self.make_client(std::move(service_locator_client)));
    }, call_guard<GilRelease>()).
    def("close", &RiskServiceTestEnvironment::close, call_guard<GilRelease>());
}

void Nexus::Python::export_risk_state(module& module) {
  auto risk_state = class_<RiskState>(module, "RiskState").
    def(init()).
    def(init<const RiskState&>()).
    def(init<RiskState::Type>()).
    def(init<RiskState::Type, boost::posix_time::ptime>()).
    def_readwrite("type", &RiskState::m_type).
    def_readwrite("expiry", &RiskState::m_expiry).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, RiskState>);
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
        MakeSqlConnection(Viper::Sqlite3::Connection(std::move(path))));
    }));
}
