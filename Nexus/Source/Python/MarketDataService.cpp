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
#include "Nexus/MarketDataService/VirtualHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/HistoricalDataStore.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonMarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Python;
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
  auto marketDataClientBox = std::unique_ptr<class_<MarketDataClientBox>>();
  auto marketDataFeedClientBox =
    std::unique_ptr<class_<MarketDataFeedClientBox>>();

  struct TrampolineHistoricalDataStore final : VirtualHistoricalDataStore {
    optional<SecurityInfo> LoadSecurityInfo(const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(optional<SecurityInfo>,
        VirtualHistoricalDataStore, "load_security_info", LoadSecurityInfo,
        security);
    }

    std::vector<SecurityInfo> LoadAllSecurityInfo() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SecurityInfo>,
        VirtualHistoricalDataStore, "load_all_security_info",
        LoadAllSecurityInfo);
    }

    std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SequencedOrderImbalance>,
        VirtualHistoricalDataStore, "load_order_imbalances",
        LoadOrderImbalances, query);
    }

    std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SequencedBboQuote>,
        VirtualHistoricalDataStore, "load_bbo_quotes", LoadBboQuotes, query);
    }

    std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SequencedBookQuote>,
        VirtualHistoricalDataStore, "load_book_quotes", LoadBookQuotes, query);
    }

    std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SequencedMarketQuote>,
        VirtualHistoricalDataStore, "load_market_quotes", LoadMarketQuotes,
        query);
    }

    std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SequencedTimeAndSale>,
        VirtualHistoricalDataStore, "load_time_and_sales", LoadTimeAndSales,
        query);
    }

    void Store(const SecurityInfo& info) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, info);
    }

    void Store(const SequencedMarketOrderImbalance& orderImbalance) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, orderImbalance);
    }

    void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, orderImbalances);
    }

    void Store(const SequencedSecurityBboQuote& bboQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, bboQuote);
    }

    void Store(
        const std::vector<SequencedSecurityBboQuote>& bboQuotes) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, bboQuotes);
    }

    void Store(const SequencedSecurityMarketQuote& marketQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, marketQuote);
    }

    void Store(const std::vector<
        SequencedSecurityMarketQuote>& marketQuotes) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, marketQuotes);
    }

    void Store(const SequencedSecurityBookQuote& bookQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, bookQuote);
    }

    void Store(
        const std::vector<SequencedSecurityBookQuote>& bookQuotes) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, bookQuotes);
    }

    void Store(const SequencedSecurityTimeAndSale& timeAndSale) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, timeAndSale);
    }

    void Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "store",
        Store, timeAndSales);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "close",
        Close);
    }
  };
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
  class_<PythonApplicationMarketDataFeedClient,
    std::shared_ptr<PythonApplicationMarketDataFeedClient>>(module,
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

void Nexus::Python::ExportHistoricalDataStore(pybind11::module& module) {
  class_<VirtualHistoricalDataStore, TrampolineHistoricalDataStore,
      std::shared_ptr<VirtualHistoricalDataStore>>(module,
      "HistoricalDataStore").
    def("load_security_info", &VirtualHistoricalDataStore::LoadSecurityInfo).
    def("load_all_security_info",
      &VirtualHistoricalDataStore::LoadAllSecurityInfo).
    def("load_order_imbalances",
      &VirtualHistoricalDataStore::LoadOrderImbalances).
    def("load_bbo_quotes", &VirtualHistoricalDataStore::LoadBboQuotes).
    def("load_book_quotes", &VirtualHistoricalDataStore::LoadBookQuotes).
    def("load_market_quotes", &VirtualHistoricalDataStore::LoadMarketQuotes).
    def("load_time_and_sales", &VirtualHistoricalDataStore::LoadTimeAndSales).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SecurityInfo&)>(&VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedMarketOrderImbalance&)>(
      &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedMarketOrderImbalance>&)>(
      &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBboQuote&)>(&VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBboQuote>&)>(
      &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityMarketQuote&)>(
        &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityMarketQuote>&)>(
      &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBookQuote&)>(&VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBookQuote>&)>(
      &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityTimeAndSale&)>(
        &VirtualHistoricalDataStore::Store)).
    def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityTimeAndSale>&)>(
      &VirtualHistoricalDataStore::Store)).
    def("close", &VirtualHistoricalDataStore::Close);
}

void Nexus::Python::ExportMarketDataReactors(pybind11::module& module) {
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

void Nexus::Python::ExportMarketDataService(pybind11::module& module) {
  auto submodule = module.def_submodule("market_data_service");
  ExportHistoricalDataStore(submodule);
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
  auto testModule = submodule.def_submodule("tests");
  ExportMarketDataServiceTestEnvironment(testModule);
}

void Nexus::Python::ExportMarketDataServiceTestEnvironment(
    pybind11::module& module) {
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

void Nexus::Python::ExportMySqlHistoricalDataStore(pybind11::module& module) {
  using PythonHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::MySql::Connection>>>;
  class_<PythonHistoricalDataStore, VirtualHistoricalDataStore,
      std::shared_ptr<PythonHistoricalDataStore>>(module,
      "MySqlHistoricalDataStore")
    .def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return MakeToPythonHistoricalDataStore(std::make_unique<
        SqlHistoricalDataStore<SqlConnection<Viper::MySql::Connection>>>(
        [=] {
          return SqlConnection(Viper::MySql::Connection(host, port, username,
            password, database));
        }));
    }), call_guard<GilRelease>());
}

void Nexus::Python::ExportSecuritySnapshot(pybind11::module& module) {
  class_<SecuritySnapshot>(module, "SecuritySnapshot")
    .def(init())
    .def(init<const Security&>())
    .def(init<const SecuritySnapshot&>())
    .def_readwrite("security", &SecuritySnapshot::m_security)
    .def_readwrite("bbo_quote", &SecuritySnapshot::m_bboQuote)
    .def_readwrite("time_and_sale", &SecuritySnapshot::m_timeAndSale)
    .def_readwrite("market_quotes", &SecuritySnapshot::m_marketQuotes)
    .def_readwrite("ask_book", &SecuritySnapshot::m_askBook)
    .def_readwrite("bid_book", &SecuritySnapshot::m_bidBook);
}

void Nexus::Python::ExportSqliteHistoricalDataStore(pybind11::module& module) {
  using PythonHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  class_<PythonHistoricalDataStore, VirtualHistoricalDataStore,
      std::shared_ptr<PythonHistoricalDataStore>>(module,
      "SqliteHistoricalDataStore")
    .def(init([] (std::string path) {
      return MakeToPythonHistoricalDataStore(std::make_unique<
        SqlHistoricalDataStore<SqlConnection<Viper::Sqlite3::Connection>>>(
        [=] {
          return SqlConnection(Viper::Sqlite3::Connection(path));
        }));
    }), call_guard<GilRelease>());
}
