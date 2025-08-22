#include "Nexus/Python/MarketDataService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/AsyncHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/DataStoreMarketDataClient.hpp"
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/EntitlementSet.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreException.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataType.hpp"
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonHistoricalDataStore.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonMarketDataFeedClient.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace pybind11;

void Nexus::Python::export_async_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<AsyncHistoricalDataStore<HistoricalDataStore>>;
  auto data_store =
    export_historical_data_store<DataStore>(module, "AsyncHistoricalDataStore");
  data_store.def(init<HistoricalDataStore>());
}

void Nexus::Python::export_cached_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<CachedHistoricalDataStore<HistoricalDataStore>>;
  auto data_store = export_historical_data_store<DataStore>(
    module, "CachedHistoricalDataStore");
  data_store.def(init<HistoricalDataStore, int>());
}

void Nexus::Python::export_client_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<ClientHistoricalDataStore<MarketDataClient>>;
  auto data_store = export_historical_data_store<DataStore>(
    module, "ClientHistoricalDataStore");
  data_store.def(init<MarketDataClient>());
}

void Nexus::Python::export_data_store_market_data_client(module& module) {
  using Client =
    ToPythonMarketDataClient<DataStoreMarketDataClient<HistoricalDataStore>>;
  auto client =
    export_market_data_client<Client>(module, "DataStoreMarketDataClient");
  client.def(init<HistoricalDataStore>());
}

void Nexus::Python::export_entitlement_database(module& module) {
  ExportView<EntitlementDatabase::Entry>(
    module, "EntitlementDatabaseEntryView");
  ExportView<const EntitlementDatabase::Entry>(
    module, "EntitlementDatabaseEntryConstView");
  auto database = class_<EntitlementDatabase>(module, "EntitlementDatabase").
    def(init()).
    def(init<const EntitlementDatabase&>()).
    def_property_readonly("entries", &EntitlementDatabase::get_entries).
    def("add", &EntitlementDatabase::add).
    def("remove", &EntitlementDatabase::remove);
  database.def_readonly_static("NONE", &EntitlementDatabase::NONE);
  class_<EntitlementDatabase::Entry>(module, "Entry").
    def(init()).
    def(init<const EntitlementDatabase::Entry&>()).
    def_readwrite("name", &EntitlementDatabase::Entry::m_name).
    def_readwrite("price", &EntitlementDatabase::Entry::m_price).
    def_readwrite("currency", &EntitlementDatabase::Entry::m_currency).
    def_readwrite("group_entry", &EntitlementDatabase::Entry::m_group_entry).
    def_readwrite("applicability",
      &EntitlementDatabase::Entry::m_applicability);
}

void Nexus::Python::export_entitlement_set(module& module) {
  class_<EntitlementKey>(module, "EntitlementKey").
    def(init()).
    def(init<const EntitlementKey&>()).
    def(init<Venue>()).
    def(init<Venue, Venue>()).
    def_readwrite("venue", &EntitlementKey::m_venue).
    def_readwrite("source", &EntitlementKey::m_source).
    def(self == self).
    def(self != self).
    def("__hash__",  std::hash<EntitlementKey>());
  class_<EntitlementSet>(module, "EntitlementSet").
    def(init()).
    def(init<const EntitlementSet&>()).
    def("contains", &EntitlementSet::contains).
    def("grant", &EntitlementSet::grant).
    def(self == self).
    def(self != self);
}

void Nexus::Python::export_historical_data_store_exception(module& module) {
  register_exception<HistoricalDataStoreException>(
    module, "HistoricalDataStoreException", GetIOException());
}

void Nexus::Python::export_local_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<LocalHistoricalDataStore>;
  auto data_store =
    export_historical_data_store<DataStore>(module, "LocalHistoricalDataStore");
  data_store.def(init());
  data_store.def("load_all_order_imbalances", [] (DataStore& self) {
    return self.get_data_store().load_order_imbalances();
  }, call_guard<GilRelease>());
  data_store.def("load_all_bbo_quotes", [] (DataStore& self) {
    return self.get_data_store().load_bbo_quotes();
  }, call_guard<GilRelease>());
  data_store.def("load_all_book_quotes", [] (DataStore& self) {
    return self.get_data_store().load_book_quotes();
  }, call_guard<GilRelease>());
  data_store.def("load_all_time_and_sales", [] (DataStore& self) {
    return self.get_data_store().load_time_and_sales();
  }, call_guard<GilRelease>());
}

void Nexus::Python::export_market_data_reactors(module& module) {
  auto aspen_module = pybind11::module::import("aspen");
  export_box<SecurityMarketDataQuery>(aspen_module, "SecurityMarketDataQuery");
  export_box<Security>(aspen_module, "Security");
  module.def("bbo_quote_reactor",
    [] (MarketDataClient client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(
        make_bbo_quote_reactor(std::move(client), std::move(query)));
    }, arg("client"), arg("query"));
  module.def("current_bbo_quote_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(
        make_current_bbo_quote_reactor(std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
  module.def("real_time_bbo_quote_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(make_real_time_bbo_quote_reactor(
        std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
  module.def("book_quote_reactor",
    [] (MarketDataClient client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(
        make_book_quote_reactor(std::move(client), std::move(query)));
    }, arg("client"), arg("query"));

  module.def("current_book_quote_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(make_current_book_quote_reactor(
        std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
  module.def("real_time_book_quote_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(make_real_time_book_quote_reactor(
        std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
  module.def("time_and_sales_reactor",
    [] (MarketDataClient client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(
        make_time_and_sales_reactor(std::move(client), std::move(query)));
    }, arg("client"), arg("query"));
  module.def("current_time_and_sales_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(make_current_time_and_sales_reactor(
        std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
  module.def("real_time_time_and_sales_reactor",
    [] (MarketDataClient client, SharedBox<Security> security) {
      return to_object(make_real_time_time_and_sales_reactor(
        std::move(client), std::move(security)));
    }, arg("client"), arg("security"));
}

void Nexus::Python::export_market_data_service(module& module) {
  auto submodule = module.def_submodule("market_data_service");
  export_async_historical_data_store(submodule);
  export_cached_historical_data_store(submodule);
  export_client_historical_data_store(submodule);
  export_data_store_market_data_client(submodule);
  export_entitlement_database(submodule);
  export_entitlement_set(submodule);
  export_historical_data_store<ToPythonHistoricalDataStore<
    HistoricalDataStore>>(submodule, "HistoricalDataStore");
  export_historical_data_store_exception(submodule);
  export_local_historical_data_store(submodule);
  export_market_data_client<ToPythonMarketDataClient<MarketDataClient>>(
    submodule, "MarketDataClient");
  export_market_data_reactors(submodule);
  export_market_data_type(submodule);
  export_mysql_historical_data_store(submodule);
  export_security_snapshot(submodule);
  export_sqlite_historical_data_store(submodule);
  submodule.def("query_real_time_book_quotes_with_snapshot",
    [] (MarketDataClient client, const Security& security,
        ScopedQueueWriter<BookQuote> queue,
        InterruptionPolicy interruption_policy) {
      return query_real_time_with_snapshot(
        client, security, std::move(queue), interruption_policy);
    }, arg("client"), arg("security"), arg("queue"),
    arg("interruption_policy") =
      Beam::Queries::InterruptionPolicy::BREAK_QUERY);
  submodule.def("query_real_time_bbo_quotes_with_snapshot",
    [] (MarketDataClient client, const Security& security,
        Beam::ScopedQueueWriter<BboQuote> queue) {
      return query_real_time_with_snapshot(
        std::move(client), security, std::move(queue));
    });
  submodule.def("load_security_info",
    [] (MarketDataClient& client, const Security& security) {
      return load_security_info(client, security);
    });
  auto test_module = submodule.def_submodule("tests");
  export_market_data_service_test_environment(test_module);
}

void Nexus::Python::export_market_data_service_test_environment(
    module& module) {
  using TestEnvironment = MarketDataServiceTestEnvironment;
  class_<TestEnvironment>(module, "MarketDataServiceTestEnvironment").
    def(init<ServiceLocatorClientBox, AdministrationClient>(),
      call_guard<GilRelease>()).
    def(init<ServiceLocatorClientBox, AdministrationClient,
      HistoricalDataStore>(), call_guard<GilRelease>()).
    def("__del__", [] (TestEnvironment& self) {
      self.close();
    }, call_guard<GilRelease>()).
    def_property_readonly("data_store", [] (TestEnvironment& self) {
      return ToPythonHistoricalDataStore(self.get_data_store());
    }).
    def_property_readonly("registry_client", [] (TestEnvironment& self) {
      return ToPythonMarketDataClient(self.get_registry_client());
    }).
    def_property_readonly("feed_client", [] (TestEnvironment& self) {
      return ToPythonMarketDataFeedClient(self.get_feed_client());
    }).
    def("make_registry_client", [] (TestEnvironment& self,
        ServiceLocatorClientBox service_locator_client) {
      return ToPythonMarketDataClient(
        self.make_registry_client(std::move(service_locator_client)));
    }, call_guard<GilRelease>()).
    def("make_feed_client", [] (TestEnvironment& self,
        ServiceLocatorClientBox service_locator_client) {
      return ToPythonMarketDataFeedClient(
        self.make_feed_client(std::move(service_locator_client)));
    }, call_guard<GilRelease>()).
    def("update_bbo", static_cast<void (TestEnvironment::*)(
      const Security&, Money, Money)>(&TestEnvironment::update_bbo),
      call_guard<GilRelease>()).
    def("update_bbo", static_cast<void (TestEnvironment::*)(
      const Security&, Money)>(&TestEnvironment::update_bbo),
      call_guard<GilRelease>()).
    def("close", &TestEnvironment::close, call_guard<GilRelease>());
  module.def("make_market_data_service_test_environment",
    &make_market_data_service_test_environment, call_guard<GilRelease>());
  module.def("make_market_data_client", &make_market_data_client,
    call_guard<GilRelease>());
}

void Nexus::Python::export_market_data_type(module& module) {
  enum_<MarketDataType::Type>(module, "MarketDataType").
    value("TIME_AND_SALE", MarketDataType::TIME_AND_SALE).
    value("BOOK_QUOTE", MarketDataType::BOOK_QUOTE).
    value("BBO_QUOTE", MarketDataType::BBO_QUOTE).
    value("ORDER_IMBALANCE", MarketDataType::ORDER_IMBALANCE);
  ExportEnumSet<MarketDataTypeSet>(module, "MarketDataTypeSet");
}

void Nexus::Python::export_mysql_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_historical_data_store<DataStore>(module, "MySqlHistoricalDataStore").
    def(init([] (const VenueDatabase& venues, std::string host,
        unsigned int port, std::string username, std::string password,
        std::string database) {
      return std::make_shared<DataStore>(venues, [=] {
        auto release = Beam::Python::GilRelease();
        return SqlConnection(
          Viper::MySql::Connection(host, port, username, password, database));
      });
    })).
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_shared<DataStore>(DEFAULT_VENUES, [=] {
        auto release = Beam::Python::GilRelease();
        return SqlConnection(
          Viper::MySql::Connection(host, port, username, password, database));
      });
    }));
}

void Nexus::Python::export_security_snapshot(module& module) {
  class_<SecuritySnapshot>(module, "SecuritySnapshot").
    def(init()).
    def(init<const SecuritySnapshot&>()).
    def(init<Security>()).
    def_readwrite("security", &SecuritySnapshot::m_security).
    def_readwrite("bbo_quote", &SecuritySnapshot::m_bbo_quote).
    def_readwrite("time_and_sale", &SecuritySnapshot::m_time_and_sale).
    def_readwrite("asks", &SecuritySnapshot::m_asks).
    def_readwrite("bids", &SecuritySnapshot::m_bids).
    def(self == self).
    def(self != self);
}

void Nexus::Python::export_sqlite_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  export_historical_data_store<DataStore>(module, "SqliteHistoricalDataStore").
    def(init([] (const VenueDatabase& venues, std::string path) {
      return std::make_shared<DataStore>(venues, [=] {
        auto release = Beam::Python::GilRelease();
        return SqlConnection(Viper::Sqlite3::Connection(path));
      });
    })).
    def(init([] (std::string path) {
      return std::make_shared<DataStore>(DEFAULT_VENUES, [=] {
        auto release = Beam::Python::GilRelease();
        return SqlConnection(Viper::Sqlite3::Connection(path));
      });
    }));
}
