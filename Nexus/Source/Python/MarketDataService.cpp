#include "Nexus/Python/MarketDataService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/HistoricalDataStore.hpp"
#include "Nexus/Python/MarketDataClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineHistoricalDataStore final : VirtualHistoricalDataStore {
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

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "open",
        Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualHistoricalDataStore, "close",
        Close);
    }
  };

  struct TrampolineMarketDataClient final : VirtualMarketDataClient {
    void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedOrderImbalance>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_order_imbalances", QueryOrderImbalances, query, queue);
    }

    void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<QueueWriter<OrderImbalance>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_order_imbalances", QueryOrderImbalances, query, queue);
    }

    void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedBboQuote>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_bbo_quotes", QueryBboQuotes, query, queue);
    }

    void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<BboQuote>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_bbo_quotes", QueryBboQuotes, query, queue);
    }

    void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedBookQuote>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_book_quotes", QueryBookQuotes, query, queue);
    }

    void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<BookQuote>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_book_quotes", QueryBookQuotes, query, queue);
    }

    void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedMarketQuote>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_market_quotes", QueryMarketQuotes, query, queue);
    }

    void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<MarketQuote>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_market_quotes", QueryMarketQuotes, query, queue);
    }

    void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedTimeAndSale>>& queue)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_time_and_sales", QueryTimeAndSales, query, queue);
    }

    void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<TimeAndSale>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_time_and_sales", QueryTimeAndSales, query, queue);
    }

    SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(SecuritySnapshot, VirtualMarketDataClient,
        "load_security_snapshot", LoadSecuritySnapshot, security);
    }

    SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(SecurityTechnicals, VirtualMarketDataClient,
        "load_security_technicals", LoadSecurityTechnicals, security);
    }

    std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<SecurityInfo>,
        VirtualMarketDataClient, "load_security_info_from_prefix",
        LoadSecurityInfoFromPrefix, prefix);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient, "open", Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient, "close",
        Close);
    }
  };
}

void Nexus::Python::ExportApplicationMarketDataClient(
    pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  using Client = MarketDataClient<SessionBuilder>;
  class_<ToPythonMarketDataClient<Client>, VirtualMarketDataClient>(
      module, "ApplicationMarketDataClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          MarketDataService::RELAY_SERVICE_NAME);
        auto delay = false;
        auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
          [=] () mutable {
            if(delay) {
              auto delayTimer = LiveTimer(seconds(3),
                Ref(*GetTimerThreadPool()));
              delayTimer.Start();
              delayTimer.Wait();
            }
            delay = true;
            return std::make_unique<TcpSocketChannel>(addresses,
              Ref(*GetSocketThreadPool()));
          },
          [=] {
            return std::make_unique<LiveTimer>(seconds(10),
              Ref(*GetTimerThreadPool()));
          });
        return MakeToPythonMarketDataClient(std::make_unique<Client>(
          sessionBuilder));
      }));
}

void Nexus::Python::ExportHistoricalDataStore(pybind11::module& module) {
  class_<VirtualHistoricalDataStore, TrampolineHistoricalDataStore,
      std::shared_ptr<VirtualHistoricalDataStore>>(module,
    "HistoricalDataStore")
    .def("load_order_imbalances",
      &VirtualHistoricalDataStore::LoadOrderImbalances)
    .def("load_bbo_quotes", &VirtualHistoricalDataStore::LoadBboQuotes)
    .def("load_book_quotes", &VirtualHistoricalDataStore::LoadBookQuotes)
    .def("load_market_quotes", &VirtualHistoricalDataStore::LoadMarketQuotes)
    .def("load_time_and_sales", &VirtualHistoricalDataStore::LoadTimeAndSales)
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedMarketOrderImbalance&)>(
      &VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedMarketOrderImbalance>&)>(
      &VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBboQuote&)>(&VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBboQuote>&)>(
      &VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityMarketQuote&)>(&VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityMarketQuote>&)>(
      &VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBookQuote&)>(&VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBookQuote>&)>(
      &VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityTimeAndSale&)>(&VirtualHistoricalDataStore::Store))
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityTimeAndSale>&)>(
      &VirtualHistoricalDataStore::Store))
    .def("open", &VirtualHistoricalDataStore::Open)
    .def("close", &VirtualHistoricalDataStore::Close);
}

void Nexus::Python::ExportMarketDataClient(pybind11::module& module) {
  class_<VirtualMarketDataClient, TrampolineMarketDataClient>(module,
      "MarketDataClient")
    .def("query_sequenced_order_imbalances",
      static_cast<void (VirtualMarketDataClient::*)(const MarketWideDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedOrderImbalance>>&)>(
      &VirtualMarketDataClient::QueryOrderImbalances))
    .def("query_order_imbalances",
      static_cast<void (VirtualMarketDataClient::*)(const MarketWideDataQuery&,
      const std::shared_ptr<QueueWriter<OrderImbalance>>&)>(
      &VirtualMarketDataClient::QueryOrderImbalances))
    .def("query_sequenced_bbo_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedBboQuote>>&)>(
      &VirtualMarketDataClient::QueryBboQuotes))
    .def("query_bbo_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<BboQuote>>&)>(
      &VirtualMarketDataClient::QueryBboQuotes))
    .def("query_sequenced_book_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedBookQuote>>&)>(
      &VirtualMarketDataClient::QueryBookQuotes))
    .def("query_book_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<BookQuote>>&)>(
      &VirtualMarketDataClient::QueryBookQuotes))
    .def("query_sequenced_market_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedMarketQuote>>&)>(
      &VirtualMarketDataClient::QueryMarketQuotes))
    .def("query_market_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<MarketQuote>>&)>(
      &VirtualMarketDataClient::QueryMarketQuotes))
    .def("query_sequenced_time_and_sales",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedTimeAndSale>>&)>(
      &VirtualMarketDataClient::QueryTimeAndSales))
    .def("query_time_and_sales", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<TimeAndSale>>&)>(
      &VirtualMarketDataClient::QueryTimeAndSales))
    .def("load_security_snapshot",
      &VirtualMarketDataClient::LoadSecuritySnapshot)
    .def("load_security_technicals",
      &VirtualMarketDataClient::LoadSecurityTechnicals)
    .def("load_security_info_from_prefix",
      &VirtualMarketDataClient::LoadSecurityInfoFromPrefix)
    .def("open", &VirtualMarketDataClient::Open)
    .def("close", &VirtualMarketDataClient::Close);
}

void Nexus::Python::ExportMarketDataReactors(pybind11::module& module) {
  auto aspenModule = pybind11::module::import("aspen");
  Aspen::export_box<SecurityMarketDataQuery>(aspenModule,
    "SecurityMarketDataQuery");
  Aspen::export_box<Security>(aspenModule, "Security");
  module.def("bbo_quote_reactor",
    [] (VirtualMarketDataClient& client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(BboQuoteReactor(client, std::move(query)));
    });
  module.def("current_bbo_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentBboQuoteReactor(client,
        std::move(security)));
    });
  module.def("real_time_bbo_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeBboQuoteReactor(client,
        std::move(security)));
    });
  module.def("book_quote_reactor",
    [] (VirtualMarketDataClient& client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(BookQuoteReactor(client, std::move(query)));
    });
  module.def("current_book_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentBookQuoteReactor(client,
        std::move(security)));
    });
  module.def("real_time_book_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeBookQuoteReactor(client,
        std::move(security)));
    });
  module.def("market_quote_reactor",
    [] (VirtualMarketDataClient& client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(MarketQuoteReactor(client, std::move(query)));
    });
  module.def("current_market_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentMarketQuoteReactor(client,
        std::move(security)));
    });
  module.def("real_time_market_quote_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeMarketQuoteReactor(client,
        std::move(security)));
    });
  module.def("time_and_sales_reactor",
    [] (VirtualMarketDataClient& client,
        Aspen::SharedBox<SecurityMarketDataQuery> query) {
      return Aspen::to_object(TimeAndSalesReactor(client, std::move(query)));
    });
  module.def("current_time_and_sales_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(CurrentTimeAndSalesReactor(client,
        std::move(security)));
    });
  module.def("real_time_time_and_sales_reactor",
    [] (VirtualMarketDataClient& client, Aspen::SharedBox<Security> security) {
      return Aspen::to_object(RealTimeTimeAndSalesReactor(client,
        std::move(security)));
    });
}

void Nexus::Python::ExportMarketDataService(pybind11::module& module) {
  auto submodule = module.def_submodule("market_data_service");
  ExportHistoricalDataStore(submodule);
  ExportMarketDataClient(submodule);
  ExportApplicationMarketDataClient(submodule);
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
      "MarketDataServiceTestEnvironment")
    .def(init(
      [] (const std::shared_ptr<VirtualServiceLocatorClient>&
          serviceLocatorClient,
          const std::shared_ptr<VirtualAdministrationClient>&
          administrationClient) {
        return std::make_unique<MarketDataServiceTestEnvironment>(
          serviceLocatorClient, administrationClient);
      }))
    .def("open", &MarketDataServiceTestEnvironment::Open,
      call_guard<GilRelease>())
    .def("close", &MarketDataServiceTestEnvironment::Close,
      call_guard<GilRelease>())
    .def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(
      &MarketDataServiceTestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const BboQuote&)>(
      &MarketDataServiceTestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const BookQuote&)>(
      &MarketDataServiceTestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const MarketQuote&)>(
      &MarketDataServiceTestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("publish", static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const TimeAndSale&)>(
      &MarketDataServiceTestEnvironment::Publish),
      call_guard<GilRelease>())
    .def("build_client",
      [] (MarketDataServiceTestEnvironment& self,
          VirtualServiceLocatorClient& serviceLocatorClient) {
        return MakeToPythonMarketDataClient(self.BuildClient(
          Ref(serviceLocatorClient)));
      });
}

void Nexus::Python::ExportMySqlHistoricalDataStore(pybind11::module& module) {
  class_<ToPythonHistoricalDataStore<SqlHistoricalDataStore<
      Viper::MySql::Connection>>, VirtualHistoricalDataStore,
      std::shared_ptr<ToPythonHistoricalDataStore<SqlHistoricalDataStore<
      Viper::MySql::Connection>>>>(module,
      "MySqlHistoricalDataStore")
    .def(init(
      [] (std::string host, unsigned int port, std::string username,
          std::string password, std::string database) {
        return MakeToPythonHistoricalDataStore(
          std::make_unique<SqlHistoricalDataStore<Viper::MySql::Connection>>(
          [=] {
            return Viper::MySql::Connection(host, port, username, password,
              database);
          }));
      }));
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
  using PythonSqliteHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<Viper::Sqlite3::Connection>>;
  class_<PythonSqliteHistoricalDataStore, VirtualHistoricalDataStore,
      std::shared_ptr<PythonSqliteHistoricalDataStore>>(module,
      "SqliteHistoricalDataStore")
    .def(init(
      [] (std::string path) {
        return MakeToPythonHistoricalDataStore(
          std::make_unique<SqlHistoricalDataStore<Viper::Sqlite3::Connection>>(
          [=] {
            return Viper::Sqlite3::Connection(path);
          }));
      }));
}
