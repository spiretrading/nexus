#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Python/RiskDataStore.hpp"
#include "Nexus/Python/ToPythonRiskClient.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Nexus/RiskServiceTests/RiskServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Nexus::RiskService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto riskClientBox = std::unique_ptr<class_<RiskClientBox>>();

  struct TrampolineRiskDataStore final : VirtualRiskDataStore {
    InventorySnapshot LoadInventorySnapshot(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(InventorySnapshot, VirtualRiskDataStore,
        "load_inventory_snapshot", LoadInventorySnapshot, account);
    }

    void Store(const DirectoryEntry& account,
        const InventorySnapshot& snapshot) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskDataStore, "store", Store,
        account, snapshot);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskDataStore, "close", Close);
    }
  };
}

class_<RiskClientBox>& Nexus::Python::GetExportedRiskClientBox() {
  return *riskClientBox;
}

void Nexus::Python::ExportApplicationRiskClient(module& module) {
  using PythonApplicationRiskClient = ToPythonRiskClient<
    RiskClient<DefaultSessionBuilder<ServiceLocatorClientBox>>>;
  ExportRiskClient<PythonApplicationRiskClient>(module,
    "ApplicationRiskClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationRiskClient>(
        MakeDefaultSessionBuilder(std::move(serviceLocatorClient),
          RiskService::SERVICE_NAME));
    }));
}

void Nexus::Python::ExportInventorySnapshot(module& module) {
  class_<InventorySnapshot>(module, "InventorySnapshot")
    .def(init())
    .def(init<const InventorySnapshot&>())
    .def(init<std::vector<RiskInventory>, Beam::Queries::Sequence,
      std::vector<OrderId>>())
    .def_readwrite("inventories", &InventorySnapshot::m_inventories)
    .def_readwrite("sequence", &InventorySnapshot::m_sequence)
    .def_readwrite("excluded_orders", &InventorySnapshot::m_excludedOrders)
    .def(self == self)
    .def(self != self);
  module.def("make_portfolio", &MakePortfolio<OrderExecutionClientBox>);
}

void Nexus::Python::ExportLocalRiskDataStore(module& module) {
  class_<ToPythonRiskDataStore<LocalRiskDataStore>, VirtualRiskDataStore,
      std::shared_ptr<ToPythonRiskDataStore<LocalRiskDataStore>>>(module,
      "LocalRiskDataStore")
    .def(init([] {
      return MakeToPythonRiskDataStore(std::make_unique<LocalRiskDataStore>());
    }));
}

void Nexus::Python::ExportMySqlRiskDataStore(module& module) {
  class_<ToPythonRiskDataStore<
      SqlRiskDataStore<SqlConnection<Viper::MySql::Connection>>>,
      VirtualRiskDataStore, std::shared_ptr<ToPythonRiskDataStore<
        SqlRiskDataStore<SqlConnection<Viper::MySql::Connection>>>>>(module,
          "MySqlRiskDataStore")
    .def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return MakeToPythonRiskDataStore(std::make_unique<SqlRiskDataStore<
        SqlConnection<Viper::MySql::Connection>>>(MakeSqlConnection(
          Viper::MySql::Connection(std::move(host), port, std::move(username),
            std::move(password), std::move(database)))));
    }), call_guard<GilRelease>());
}

void Nexus::Python::ExportRiskDataStore(module& module) {
  class_<VirtualRiskDataStore, TrampolineRiskDataStore,
      std::shared_ptr<VirtualRiskDataStore>>(module, "RiskDataStore")
    .def("load_inventory_snapshot",
      &VirtualRiskDataStore::LoadInventorySnapshot)
    .def("store", &VirtualRiskDataStore::Store)
    .def("close", &VirtualRiskDataStore::Close);
}

void Nexus::Python::ExportRiskParameters(module& module) {
  class_<RiskParameters>(module, "RiskParameters")
    .def(init())
    .def(init<const RiskParameters&>())
    .def(init<CurrencyId, Money, RiskState, Money, int, time_duration>())
    .def_readwrite("currency", &RiskParameters::m_currency)
    .def_readwrite("buying_power", &RiskParameters::m_buyingPower)
    .def_readwrite("allowed_state", &RiskParameters::m_allowedState)
    .def_readwrite("net_loss", &RiskParameters::m_netLoss)
    .def_readwrite("loss_from_top", &RiskParameters::m_lossFromTop)
    .def_readwrite("transition_time", &RiskParameters::m_transitionTime)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportRiskService(module& module) {
  auto submodule = module.def_submodule("risk_service");
  riskClientBox = std::make_unique<class_<RiskClientBox>>(
    ExportRiskClient<RiskClientBox>(submodule, "RiskClient"));
  ExportRiskClient<ToPythonRiskClient<RiskClientBox>>(submodule,
    "RiskClientBox");
  ExportApplicationRiskClient(submodule);
  ExportInventorySnapshot(submodule);
  ExportRiskDataStore(submodule);
  ExportLocalRiskDataStore(submodule);
  ExportMySqlRiskDataStore(submodule);
  ExportRiskParameters(submodule);
  ExportRiskState(submodule);
  ExportSqliteRiskDataStore(submodule);
  ExportQueueSuite<KeyValuePair<RiskPortfolioKey, RiskInventory>>(module,
    "RiskPortfolioUpdateEntry");
  auto testModule = submodule.def_submodule("tests");
  ExportRiskServiceTestEnvironment(testModule);
}

void Nexus::Python::ExportRiskServiceTestEnvironment(module& module) {
  class_<RiskServiceTestEnvironment>(module, "RiskServiceTestEnvironment")
    .def(init([] (ServiceLocatorClientBox serviceLocatorClient,
          AdministrationClientBox administrationClient,
          MarketDataClientBox marketDataClient,
          OrderExecutionClientBox orderExecutionClient,
          std::function<std::shared_ptr<TimerBox> ()> transitionTimerFactory,
          TimeClientBox timeClient, std::vector<ExchangeRate> exchangeRates,
          MarketDatabase markets, DestinationDatabase destinations) {
        auto adaptedTransitionTimerFactory = [=] {
          return std::make_unique<TimerBox>(transitionTimerFactory());
        };
        return std::make_unique<RiskServiceTestEnvironment>(
          std::move(serviceLocatorClient), std::move(administrationClient),
          std::move(marketDataClient), std::move(orderExecutionClient),
          std::move(adaptedTransitionTimerFactory), std::move(timeClient),
          std::move(exchangeRates), std::move(markets),
          std::move(destinations));
      }), call_guard<GilRelease>())
    .def("__del__", [] (RiskServiceTestEnvironment& self) {
      self.Close();
    }, call_guard<GilRelease>())
    .def("make_client",
      [] (RiskServiceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonRiskClient(self.MakeClient(
          std::move(serviceLocatorClient)));
      }, call_guard<GilRelease>())
    .def("close", &RiskServiceTestEnvironment::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportRiskState(module& module) {
  auto outer = class_<RiskState>(module, "RiskState")
    .def(init())
    .def(init<RiskState::Type>())
    .def(init<RiskState::Type, ptime>())
    .def(init<const RiskState&>())
    .def_readwrite("type", &RiskState::m_type)
    .def_readwrite("expiry", &RiskState::m_expiry)
    .def("__str__", &lexical_cast<std::string, RiskState>)
    .def(self == self)
    .def(self != self);
  enum_<RiskState::Type::Type>(outer, "Type")
    .value("NONE", RiskState::Type::NONE)
    .value("ACTIVE", RiskState::Type::ACTIVE)
    .value("CLOSE_ORDERS", RiskState::Type::CLOSE_ORDERS)
    .value("DISABLED", RiskState::Type::DISABLED);
}

void Nexus::Python::ExportSqliteRiskDataStore(module& module) {
  class_<ToPythonRiskDataStore<
      SqlRiskDataStore<SqlConnection<Viper::Sqlite3::Connection>>>,
      VirtualRiskDataStore, std::shared_ptr<ToPythonRiskDataStore<
        SqlRiskDataStore<SqlConnection<Viper::Sqlite3::Connection>>>>>(module,
          "SqliteRiskDataStore")
    .def(init([] (std::string path) {
      return MakeToPythonRiskDataStore(std::make_unique<
        SqlRiskDataStore<SqlConnection<Viper::Sqlite3::Connection>>>(
          MakeSqlConnection(Viper::Sqlite3::Connection(path))));
    }), call_guard<GilRelease>());
}
