#include "Nexus/Python/MarketDataService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
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
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto historical_data_store = std::unique_ptr<class_<HistoricalDataStore>>();
  auto market_data_client = std::unique_ptr<class_<MarketDataClient>>();
  auto market_data_feed_client =
    std::unique_ptr<class_<MarketDataFeedClient>>();
}

class_<HistoricalDataStore>&
    Nexus::Python::get_exported_historical_data_store() {
  return *historical_data_store;
}

class_<MarketDataClient>& Nexus::Python::get_exported_market_data_client() {
  return *market_data_client;
}

class_<MarketDataFeedClient>&
    Nexus::Python::get_exported_market_data_feed_client() {
  return *market_data_feed_client;
}

void Nexus::Python::export_async_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<AsyncHistoricalDataStore<HistoricalDataStore>>;
  export_historical_data_store<DataStore>(module, "AsyncHistoricalDataStore").
    def(init<HistoricalDataStore&>(), keep_alive<1, 2>());
}

void Nexus::Python::export_cached_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<CachedHistoricalDataStore<HistoricalDataStore>>;
  export_historical_data_store<DataStore>(module, "CachedHistoricalDataStore").
    def(init<HistoricalDataStore&, int>(), keep_alive<1, 2>());
}

void Nexus::Python::export_client_historical_data_store(module& module) {
  using DataStore =
    ToPythonHistoricalDataStore<ClientHistoricalDataStore<MarketDataClient>>;
  export_historical_data_store<DataStore>(module, "ClientHistoricalDataStore").
    def(init<MarketDataClient&>(), keep_alive<1, 2>());
}

void Nexus::Python::export_data_store_market_data_client(module& module) {
  using Client =
    ToPythonMarketDataClient<DataStoreMarketDataClient<HistoricalDataStore>>;
  export_market_data_client<Client>(module, "DataStoreMarketDataClient").
    def(init<HistoricalDataStore&>(), keep_alive<1, 2>());
}

void Nexus::Python::export_entitlement_database(module& module) {
  export_view<EntitlementDatabase::Entry>(
    module, "EntitlementDatabaseEntryView");
  export_view<const EntitlementDatabase::Entry>(
    module, "EntitlementDatabaseEntryConstView");
  auto database = export_default_methods(
      class_<EntitlementDatabase>(module, "EntitlementDatabase")).
    def_property_readonly("entries", &EntitlementDatabase::get_entries).
    def("add", &EntitlementDatabase::add).
    def("remove", &EntitlementDatabase::remove).
    def_readonly_static("NONE", &EntitlementDatabase::NONE);
  export_default_methods(class_<EntitlementDatabase::Entry>(database, "Entry")).
    def_readwrite("name", &EntitlementDatabase::Entry::m_name).
    def_readwrite("price", &EntitlementDatabase::Entry::m_price).
    def_readwrite("currency", &EntitlementDatabase::Entry::m_currency).
    def_readwrite("group_entry", &EntitlementDatabase::Entry::m_group_entry).
    def_readwrite(
      "applicability", &EntitlementDatabase::Entry::m_applicability);
}

void Nexus::Python::export_entitlement_set(module& module) {
  export_default_methods(class_<EntitlementKey>(module, "EntitlementKey")).
    def(init<Venue>()).
    def(init<Venue, Venue>()).
    def_readwrite("venue", &EntitlementKey::m_venue).
    def_readwrite("source", &EntitlementKey::m_source);
  export_default_methods(class_<EntitlementSet>(module, "EntitlementSet")).
    def("contains", &EntitlementSet::contains).
    def("grant", &EntitlementSet::grant);
}

void Nexus::Python::export_historical_data_store_exception(module& module) {
  register_exception<HistoricalDataStoreException>(
    module, "HistoricalDataStoreException", get_io_exception());
}

void Nexus::Python::export_local_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<LocalHistoricalDataStore>;
  export_historical_data_store<DataStore>(module, "LocalHistoricalDataStore").
    def(init()).
    def("load_all_order_imbalances", [] (DataStore& self) {
      return self.get().load_order_imbalances();
    }, call_guard<gil_scoped_release>()).
    def("load_all_bbo_quotes", [] (DataStore& self) {
      return self.get().load_bbo_quotes();
    }, call_guard<gil_scoped_release>()).
    def("load_all_book_quotes", [] (DataStore& self) {
      return self.get().load_book_quotes();
    }, call_guard<gil_scoped_release>()).
    def("load_all_time_and_sales", [] (DataStore& self) {
      return self.get().load_time_and_sales();
    }, call_guard<gil_scoped_release>());
}

void Nexus::Python::export_market_data_reactors(module& module) {
  auto aspen_module = pybind11::module::import("aspen");
  export_box<SecurityMarketDataQuery>(aspen_module, "SecurityMarketDataQuery");
  export_box<Security>(aspen_module, "Security");
  module.def("bbo_quote_reactor",
    [] (MarketDataClient& client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(make_bbo_quote_reactor(client, std::move(query)));
    }, keep_alive<0, 1>());
  module.def("current_bbo_quote_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_current_bbo_quote_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
  module.def("real_time_bbo_quote_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_real_time_bbo_quote_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
  module.def("book_quote_reactor",
    [] (MarketDataClient& client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(make_book_quote_reactor(client, std::move(query)));
    }, keep_alive<0, 1>());
  module.def("current_book_quote_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_current_book_quote_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
  module.def("real_time_book_quote_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_real_time_book_quote_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
  module.def("time_and_sales_reactor",
    [] (MarketDataClient& client, SharedBox<SecurityMarketDataQuery> query) {
      return to_object(make_time_and_sales_reactor(client, std::move(query)));
    }, keep_alive<0, 1>());
  module.def("current_time_and_sales_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_current_time_and_sales_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
  module.def("real_time_time_and_sales_reactor",
    [] (MarketDataClient& client, SharedBox<Security> security) {
      return to_object(
        make_real_time_time_and_sales_reactor(client, std::move(security)));
    }, keep_alive<0, 1>());
}

void Nexus::Python::export_market_data_service(module& module) {
  historical_data_store = std::make_unique<class_<HistoricalDataStore>>(
    export_historical_data_store<HistoricalDataStore>(
      module, "HistoricalDataStore"));
  market_data_client = std::make_unique<class_<MarketDataClient>>(
    export_market_data_client<MarketDataClient>(module, "MarketDataClient"));
  market_data_feed_client = std::make_unique<class_<MarketDataFeedClient>>(
    export_market_data_feed_client<MarketDataFeedClient>(
      module, "MarketDataFeedClient"));
  export_market_data_service_application_definitions(module);
  export_async_historical_data_store(module);
  export_cached_historical_data_store(module);
  export_client_historical_data_store(module);
  export_data_store_market_data_client(module);
  export_entitlement_database(module);
  export_entitlement_set(module);
  export_historical_data_store_exception(module);
  export_local_historical_data_store(module);
  export_market_data_reactors(module);
  export_market_data_type(module);
  export_mysql_historical_data_store(module);
  export_security_snapshot(module);
  export_sqlite_historical_data_store(module);
  module.def("query_real_time_book_quotes_with_snapshot",
    [] (MarketDataClient& client, const Security& security,
        ScopedQueueWriter<BookQuote> queue,
        InterruptionPolicy interruption_policy) {
      return query_real_time_with_snapshot(
        client, security, std::move(queue), interruption_policy);
    }, arg("client"), arg("security"), arg("queue"),
    arg("interruption_policy") = InterruptionPolicy::BREAK_QUERY);
  module.def("query_real_time_bbo_quotes_with_snapshot",
    [] (MarketDataClient& client, const Security& security,
        ScopedQueueWriter<BboQuote> queue) {
      return query_real_time_with_snapshot(client, security, std::move(queue));
    });
  module.def("load_security_info",
    [] (MarketDataClient& client, const Security& security) {
      return load_security_info(client, security);
    });
  auto test_module = module.def_submodule("tests");
  export_market_data_service_test_environment(test_module);
}

void Nexus::Python::export_market_data_service_application_definitions(
    module& module) {
  export_market_data_client<
    ToPythonMarketDataClient<ApplicationMarketDataClient>>(
      module, "ApplicationMarketDataClient").
    def(init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>&
          client) {
        return std::make_unique<ToPythonMarketDataClient<
          ApplicationMarketDataClient>>(Ref(client.get()));
      }), keep_alive<1, 2>());
  export_market_data_feed_client<
    ToPythonMarketDataFeedClient<ApplicationMarketDataFeedClient>>(
      module, "ApplicationMarketDataFeedClient").
    def(init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>& client,
          boost::posix_time::time_duration sampling_time, CountryCode country) {
        return std::make_unique<ToPythonMarketDataFeedClient<
          ApplicationMarketDataFeedClient>>(
            Ref(client.get()), sampling_time, country);
      }), keep_alive<1, 2>()).
    def(init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>& client,
          boost::posix_time::time_duration sampling_time) {
        return std::make_unique<ToPythonMarketDataFeedClient<
          ApplicationMarketDataFeedClient>>(Ref(client.get()), sampling_time);
      }), keep_alive<1, 2>());
}

void Nexus::Python::export_market_data_service_test_environment(
    module& module) {
  using TestEnvironment = MarketDataServiceTestEnvironment;
  class_<TestEnvironment, std::shared_ptr<TestEnvironment>>(
      module, "MarketDataServiceTestEnvironment").
    def(init(&make_python_shared<TestEnvironment, ServiceLocatorClient&,
      AdministrationClient&>), keep_alive<1, 2>(), keep_alive<1, 3>()).
    def(init(&make_python_shared<TestEnvironment, ServiceLocatorClient&,
      AdministrationClient&, HistoricalDataStore&>), keep_alive<1, 2>(),
      keep_alive<1, 3>(), keep_alive<1, 4>()).
    def_property_readonly("data_store", [] (TestEnvironment& self) {
      return ToPythonHistoricalDataStore(self.get_data_store());
    }).
    def_property_readonly("registry_client", [] (TestEnvironment& self) {
      return ToPythonMarketDataClient(self.get_registry_client());
    }).
    def_property_readonly("feed_client", [] (TestEnvironment& self) {
      return ToPythonMarketDataFeedClient(self.get_feed_client());
    }).
    def("make_registry_client",
      [] (TestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonMarketDataClient(self.make_registry_client(Ref(client)));
      }, call_guard<gil_scoped_release>(), keep_alive<0, 2>()).
    def("make_feed_client",
      [] (TestEnvironment& self, ServiceLocatorClient& client) {
        return ToPythonMarketDataFeedClient(self.make_feed_client(Ref(client)));
      }, call_guard<gil_scoped_release>(), keep_alive<0, 2>()).
    def("update_bbo", overload_cast<const Security&, Money, Money>(
      &TestEnvironment::update_bbo), call_guard<gil_scoped_release>()).
    def("update_bbo", overload_cast<const Security&, Money>(
      &TestEnvironment::update_bbo), call_guard<gil_scoped_release>()).
    def("close", &TestEnvironment::close, call_guard<gil_scoped_release>());
  module.def("make_market_data_service_test_environment",
    &make_market_data_service_test_environment,
    call_guard<gil_scoped_release>());
  module.def("make_market_data_client", &make_market_data_client,
    call_guard<gil_scoped_release>());
}

void Nexus::Python::export_market_data_type(module& module) {
  enum_<MarketDataType::Type>(module, "MarketDataType").
    value("TIME_AND_SALE", MarketDataType::TIME_AND_SALE).
    value("BOOK_QUOTE", MarketDataType::BOOK_QUOTE).
    value("BBO_QUOTE", MarketDataType::BBO_QUOTE).
    value("ORDER_IMBALANCE", MarketDataType::ORDER_IMBALANCE);
  export_enum_set<MarketDataTypeSet>(module, "MarketDataTypeSet");
}

void Nexus::Python::export_mysql_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_historical_data_store<DataStore>(module, "MySqlHistoricalDataStore").
    def(init([] (const VenueDatabase& venues, std::string host,
        unsigned int port, std::string username, std::string password,
        std::string database) {
      return std::make_unique<DataStore>(venues, [=] {
        auto release = gil_scoped_release();
        return SqlConnection(
          Viper::MySql::Connection(host, port, username, password, database));
      });
    })).
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_unique<DataStore>(DEFAULT_VENUES, [=] {
        auto release = gil_scoped_release();
        return SqlConnection(
          Viper::MySql::Connection(host, port, username, password, database));
      });
    }));
}

void Nexus::Python::export_security_snapshot(module& module) {
  export_default_methods(class_<SecuritySnapshot>(module, "SecuritySnapshot")).
    def(init<Security>()).
    def_readwrite("security", &SecuritySnapshot::m_security).
    def_readwrite("bbo_quote", &SecuritySnapshot::m_bbo_quote).
    def_readwrite("time_and_sale", &SecuritySnapshot::m_time_and_sale).
    def_readwrite("asks", &SecuritySnapshot::m_asks).
    def_readwrite("bids", &SecuritySnapshot::m_bids);
}

void Nexus::Python::export_sqlite_historical_data_store(module& module) {
  using DataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  export_historical_data_store<DataStore>(module, "SqliteHistoricalDataStore").
    def(init([] (const VenueDatabase& venues, std::string path) {
      return std::make_unique<DataStore>(venues, [=] {
        auto release = gil_scoped_release();
        return SqlConnection(Viper::Sqlite3::Connection(path));
      });
    })).
    def(init([] (std::string path) {
      return std::make_unique<DataStore>(DEFAULT_VENUES, [=] {
        auto release = gil_scoped_release();
        return SqlConnection(Viper::Sqlite3::Connection(path));
      });
    }));
}
