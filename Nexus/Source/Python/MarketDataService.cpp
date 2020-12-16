#include "Nexus/Python/MarketDataService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/ApplicationDefinitions.hpp"
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonHistoricalDataStore.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  auto historicalDataStoreBox =
    std::unique_ptr<class_<HistoricalDataStoreBox>>();
  auto marketDataClientBox = std::unique_ptr<class_<MarketDataClientBox>>();
  auto marketDataFeedClientBox =
    std::unique_ptr<class_<MarketDataFeedClientBox>>();
}

class_<HistoricalDataStoreBox>&
    Nexus::Python::GetExportedHistoricalDataStoreBox() {
  return *historicalDataStoreBox;
}

class_<MarketDataClientBox>& Nexus::Python::GetExportedMarketDataClientBox() {
  return *marketDataClientBox;
}

class_<MarketDataFeedClientBox>&
    Nexus::Python::GetExportedMarketDataFeedClientBox() {
  return *marketDataFeedClientBox;
}

void Nexus::Python::ExportApplicationMarketDataClient(module& module) {
  using PythonApplicationMarketDataClient = ToPythonMarketDataClient<
    MarketDataClient<ZLibSessionBuilder<ServiceLocatorClientBox>>>;
  ExportMarketDataClient<PythonApplicationMarketDataClient>(module,
    "ApplicationMarketDataClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationMarketDataClient>(
        MakeSessionBuilder<ZLibSessionBuilder<ServiceLocatorClientBox>>(
          std::move(serviceLocatorClient),
          MarketDataService::RELAY_SERVICE_NAME));
    }));
}

void Nexus::Python::ExportApplicationMarketDataFeedClient(module& module) {
  using PythonApplicationMarketDataFeedClient =
    ToPythonMarketDataFeedClient<ApplicationMarketDataFeedClient::Client>;
  static auto factory = [] (ServiceLocatorClientBox serviceLocatorClient,
      time_duration sampling, CountryCode country) {
    auto service = [&] {
      if(country == CountryCode::NONE) {
        auto services = serviceLocatorClient.Locate(FEED_SERVICE_NAME);
        if(services.empty()) {
          BOOST_THROW_EXCEPTION(ConnectException(
            "No market data services available."));
        }
        return services.front();
      } else {
        if(auto service = FindMarketDataFeedService(country,
            serviceLocatorClient)) {
          return *service;
        }
        BOOST_THROW_EXCEPTION(ConnectException(
          "No market data services available."));
      }
    }();
    auto addresses = Parse<std::vector<IpAddress>>(get<std::string>(
      service.GetProperties().At("addresses")));
    return std::make_shared<PythonApplicationMarketDataFeedClient>(
      Initialize(addresses),
      SessionAuthenticator(std::move(serviceLocatorClient)),
      Initialize(sampling), Initialize(seconds(10)));
  };
  ExportMarketDataFeedClient<PythonApplicationMarketDataFeedClient>(module,
      "ApplicationMarketDataFeedClient").
    def(init(
      [] (ServiceLocatorClientBox serviceLocatorClient, time_duration sampling,
          CountryCode country) {
        return factory(std::move(serviceLocatorClient), sampling, country);
      })).
    def(init(
      [] (ServiceLocatorClientBox serviceLocatorClient, CountryCode country) {
        return factory(std::move(serviceLocatorClient), milliseconds(10),
          country);
      })).
    def(init(
      [] (ServiceLocatorClientBox serviceLocatorClient,
          time_duration sampling) {
        return factory(std::move(serviceLocatorClient), sampling,
          CountryCode::NONE);
      })).
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return factory(std::move(serviceLocatorClient), milliseconds(10),
        CountryCode::NONE);
    }));
}

void Nexus::Python::ExportMarketDataReactors(module& module) {
  auto aspenModule = pybind11::module::import("aspen");
  Aspen::export_box<SecurityMarketDataQuery>(aspenModule,
    "SecurityMarketDataQuery");
  Aspen::export_box<Security>(aspenModule, "Security");
  module.def("bbo_quote_reactor",
    [] (MarketDataClientBox client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(BboQuoteReactor(std::move(client),
        std::move(query)));
    });
  module.def("current_bbo_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentBboQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("real_time_bbo_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeBboQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("book_quote_reactor",
    [] (MarketDataClientBox client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(BookQuoteReactor(std::move(client),
        std::move(query)));
    });
  module.def("current_book_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentBookQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("real_time_book_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeBookQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("market_quote_reactor",
    [] (MarketDataClientBox client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(MarketQuoteReactor(std::move(client),
        std::move(query)));
    });
  module.def("current_market_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentMarketQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("real_time_market_quote_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeMarketQuoteReactor(std::move(client),
        std::move(security)));
    });
  module.def("time_and_sales_reactor",
    [] (MarketDataClientBox client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(TimeAndSalesReactor(std::move(client),
        std::move(query)));
    });
  module.def("current_time_and_sales_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentTimeAndSalesReactor(std::move(client),
        std::move(security)));
    });
  module.def("real_time_time_and_sales_reactor",
    [] (MarketDataClientBox client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeTimeAndSalesReactor(std::move(client),
        std::move(security)));
    });
}

void Nexus::Python::ExportMarketDataService(module& module) {
  auto submodule = module.def_submodule("market_data_service");
  historicalDataStoreBox = std::make_unique<class_<HistoricalDataStoreBox>>(
    ExportHistoricalDataStore<HistoricalDataStoreBox>(submodule,
      "HistoricalDataStore"));
  marketDataClientBox = std::make_unique<class_<MarketDataClientBox>>(
    ExportMarketDataClient<MarketDataClientBox>(submodule, "MarketDataClient"));
  ExportMarketDataClient<ToPythonMarketDataClient<MarketDataClientBox>>(
    submodule, "MarketDataClientBox");
  ExportApplicationMarketDataClient(submodule);
  marketDataFeedClientBox = std::make_unique<class_<MarketDataFeedClientBox>>(
    ExportMarketDataFeedClient<MarketDataFeedClientBox>(submodule,
      "MarketDataFeedClient"));
  ExportMarketDataFeedClient<
    ToPythonMarketDataFeedClient<MarketDataFeedClientBox>>(submodule,
      "MarketDataFeedClientBox");
  ExportApplicationMarketDataFeedClient(submodule);
  ExportSecuritySnapshot(submodule);
  ExportMarketDataReactors(submodule);
  ExportMySqlHistoricalDataStore(submodule);
  ExportSqliteHistoricalDataStore(submodule);
  submodule.def("query_real_time_with_snapshot",
    [] (Security security, MarketDataClientBox client,
        ScopedQueueWriter<BboQuote> queue) {
      return QueryRealTimeWithSnapshot(security, std::move(client),
        std::move(queue));
    });
  submodule.def("query_real_time_book_quotes_with_snapshot",
    [] (MarketDataClientBox marketDataClient, Security security,
        ScopedQueueWriter<BookQuote> queue,
        InterruptionPolicy interruptionPolicy) {
      QueryRealTimeBookQuotesWithSnapshot(std::move(marketDataClient),
        std::move(security), std::move(queue), interruptionPolicy);
    });
  submodule.def("query_real_time_book_quotes_with_snapshot",
    [] (MarketDataClientBox marketDataClient, Security security,
        ScopedQueueWriter<BookQuote> queue) {
      return QueryRealTimeBookQuotesWithSnapshot(std::move(marketDataClient),
        std::move(security), std::move(queue));
    });
  submodule.def("query_real_time_market_quotes_with_snapshot",
    [] (MarketDataClientBox marketDataClient, Security security,
        ScopedQueueWriter<MarketQuote> queue,
        InterruptionPolicy interruptionPolicy) {
      return QueryRealTimeMarketQuotesWithSnapshot(std::move(marketDataClient),
        security, std::move(queue), interruptionPolicy);
    });
  submodule.def("query_real_time_market_quotes_with_snapshot",
    [] (MarketDataClientBox marketDataClient, Security security,
        ScopedQueueWriter<MarketQuote> queue) {
      return QueryRealTimeMarketQuotesWithSnapshot(std::move(marketDataClient),
        security, std::move(queue));
    });
  auto testModule = submodule.def_submodule("tests");
  ExportMarketDataServiceTestEnvironment(testModule);
}

void Nexus::Python::ExportMarketDataServiceTestEnvironment(module& module) {
  class_<MarketDataServiceTestEnvironment>(module,
      "MarketDataServiceTestEnvironment").
    def(init(
      [] (ServiceLocatorClientBox serviceLocatorClient,
          AdministrationClientBox administrationClient) {
        return std::make_unique<MarketDataServiceTestEnvironment>(
          std::move(serviceLocatorClient), std::move(administrationClient));
      }), call_guard<GilRelease>()).
    def("__del__", [] (MarketDataServiceTestEnvironment& self) {
      self.Close();
    }, call_guard<GilRelease>()).
    def("close", &MarketDataServiceTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(
      &MarketDataServiceTestEnvironment::Publish), call_guard<GilRelease>()).
    def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const BboQuote&)>(
      &MarketDataServiceTestEnvironment::Publish), call_guard<GilRelease>()).
    def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const BookQuote&)>(
      &MarketDataServiceTestEnvironment::Publish), call_guard<GilRelease>()).
    def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const MarketQuote&)>(
      &MarketDataServiceTestEnvironment::Publish), call_guard<GilRelease>()).
    def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const TimeAndSale&)>(
      &MarketDataServiceTestEnvironment::Publish), call_guard<GilRelease>()).
    def("make_client",
      [] (MarketDataServiceTestEnvironment& self,
          ServiceLocatorClientBox serviceLocatorClient) {
        return ToPythonMarketDataClient(self.MakeClient(
          std::move(serviceLocatorClient)));
      }, call_guard<GilRelease>());
}

void Nexus::Python::ExportMySqlHistoricalDataStore(module& module) {
  using PythonHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::MySql::Connection>>>;
  ExportHistoricalDataStore<PythonHistoricalDataStore>(module,
      "MySqlHistoricalDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_shared<PythonHistoricalDataStore>([=] {
        return SqlConnection(Viper::MySql::Connection(host, port, username,
          password, database));
      });
    }), call_guard<GilRelease>());
}

void Nexus::Python::ExportSecuritySnapshot(module& module) {
  class_<SecuritySnapshot>(module, "SecuritySnapshot").
    def(init()).
    def(init<const Security&>()).
    def(init<const SecuritySnapshot&>()).
    def_readwrite("security", &SecuritySnapshot::m_security).
    def_readwrite("bbo_quote", &SecuritySnapshot::m_bboQuote).
    def_readwrite("time_and_sale", &SecuritySnapshot::m_timeAndSale).
    def_readwrite("market_quotes", &SecuritySnapshot::m_marketQuotes).
    def_readwrite("ask_book", &SecuritySnapshot::m_askBook).
    def_readwrite("bid_book", &SecuritySnapshot::m_bidBook);
}

void Nexus::Python::ExportSqliteHistoricalDataStore(module& module) {
  using PythonHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  ExportHistoricalDataStore<PythonHistoricalDataStore>(module,
      "SqliteHistoricalDataStore").
    def(init([] (std::string path) {
      return std::make_shared<PythonHistoricalDataStore>([=] {
        return SqlConnection(Viper::Sqlite3::Connection(path));
      });
    }), call_guard<GilRelease>());
}
