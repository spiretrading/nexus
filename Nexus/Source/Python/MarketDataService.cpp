#include "Nexus/Python/MarketDataService.hpp"
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/Copy.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Queries.hpp>
#include <Beam/Python/UniquePtr.hpp>
#include <Beam/Python/Vector.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonHistoricalDataStore.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"
#include "Nexus/Python/ToPythonMarketDataFeedClient.hpp"

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
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;
using namespace Nexus::Python;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<Beam::Network::TcpSocketChannel>,
    BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>>,
    LiveTimer>;
  using Client = MarketDataClient<SessionBuilder>;
  using FeedClient = MarketDataFeedClient<string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;

  struct FromPythonHistoricalDataStore final : VirtualHistoricalDataStore,
      wrapper<VirtualHistoricalDataStore> {
    std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query) override {
      return get_override("load_order_imbalances")(query);
    }

    std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query) override {
      return get_override("load_bbo_quotes")(query);
    }

    std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query) override {
      return get_override("load_book_quotes")(query);
    }

    std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query) override {
      return get_override("load_market_quotes")(query);
    }

    std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query) override {
      return get_override("load_time_and_sales")(query);
    }

    void Store(const SequencedMarketOrderImbalance& orderImbalance) override {
      get_override("store")(orderImbalance);
    }

    void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances) override {
      get_override("store")(orderImbalances);
    }

    void Store(const SequencedSecurityBboQuote& bboQuote) override {
      get_override("store")(bboQuote);
    }

    void Store(
        const std::vector<SequencedSecurityBboQuote>& bboQuotes) override {
      get_override("store")(bboQuotes);
    }

    void Store(const SequencedSecurityMarketQuote& marketQuote) override {
      get_override("store")(marketQuote);
    }

    void Store(const std::vector<
        SequencedSecurityMarketQuote>& marketQuotes) override {
      get_override("store")(marketQuotes);
    }

    void Store(const SequencedSecurityBookQuote& bookQuote) override {
      get_override("store")(bookQuote);
    }

    void Store(
        const std::vector<SequencedSecurityBookQuote>& bookQuotes) override {
      get_override("store")(bookQuotes);
    }

    void Store(const SequencedSecurityTimeAndSale& timeAndSale) override {
      get_override("store")(timeAndSale);
    }

    void Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) override {
      get_override("store")(timeAndSales);
    }

    void Open() override {
      get_override("open")();
    }

    void Close() override {
      get_override("close")();
    }
  };

  struct FromPythonMarketDataClient : VirtualMarketDataClient,
      wrapper<VirtualMarketDataClient> {
    virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedOrderImbalance>>& queue)
        override final {
      get_override("query_sequenced_order_imbalances")(query, queue);
    }

    virtual void QueryOrderImbalances(const MarketWideDataQuery& query,
        const std::shared_ptr<QueueWriter<OrderImbalance>>& queue)
        override final {
      get_override("query_order_imbalances")(query, queue);
    }

    virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedBboQuote>>& queue)
        override final {
      get_override("query_sequenced_bbo_quotes")(query, queue);
    }

    virtual void QueryBboQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<BboQuote>>& queue) override final {
      get_override("query_bbo_quotes")(query, queue);
    }

    virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedBookQuote>>& queue)
        override final {
      get_override("query_sequenced_book_quotes")(query, queue);
    }

    virtual void QueryBookQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<BookQuote>>& queue) override final {
      get_override("query_book_quotes")(query, queue);
    }

    virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedMarketQuote>>& queue)
        override final {
      get_override("query_sequenced_market_quotes")(query, queue);
    }

    virtual void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<MarketQuote>>& queue) override final {
      get_override("query_market_quotes")(query, queue);
    }

    virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<SequencedTimeAndSale>>& queue)
        override final {
      get_override("query_sequenced_time_and_sales")(query, queue);
    }

    virtual void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        const std::shared_ptr<QueueWriter<TimeAndSale>>& queue) override final {
      get_override("query_time_and_sales")(query, queue);
    }

    virtual SecuritySnapshot LoadSecuritySnapshot(
        const Security& security) override final {
      return get_override("load_security_snapshot")(security);
    }

    virtual SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override final {
      return get_override("load_security_technicals")(security);
    }

    virtual std::vector<SecurityInfo> LoadSecurityInfoFromPrefix(
        const std::string& prefix) override final {
      return get_override("load_security_info_from_prefix")(prefix);
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto BuildClient(VirtualServiceLocatorClient& serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      MarketDataService::RELAY_SERVICE_NAME);
    auto delay = false;
    auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = LiveTimer(seconds(3), Ref(*GetTimerThreadPool()));
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
      sessionBuilder)).release();
  }

  struct FromPythonMarketDataFeedClient final : VirtualMarketDataFeedClient,
      wrapper<VirtualMarketDataFeedClient> {
    void Add(const SecurityInfo& securityInfo) override {
      get_override("add")(securityInfo);
    }

    void PublishOrderImbalance(
        const MarketOrderImbalance& orderImbalance) override {
      get_override("publish_order_imbalance")(orderImbalance);
    }

    void PublishBboQuote(const SecurityBboQuote& bboQuote) override {
      get_override("publish_bbo_quote")(bboQuote);
    }

    void PublishMarketQuote(const SecurityMarketQuote& marketQuote) override {
      get_override("publish_market_quote")(marketQuote);
    }

    void SetBookQuote(const SecurityBookQuote& bookQuote) override {
      get_override("set_book_quote")(bookQuote);
    }

    void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        const boost::posix_time::ptime& timestamp) override {
      get_override("add_order")(security, market, mpid, isPrimaryMpid, id, side,
        price, size);
    }

    void ModifyOrderSize(const std::string& id, Quantity size,
        const boost::posix_time::ptime& timestamp) override {
      get_override("modify_order_size")(id, size, timestamp);
    }

    void OffsetOrderSize(const std::string& id, Quantity delta,
        const boost::posix_time::ptime& timestamp) override {
      get_override("offset_order_size")(id, delta, timestamp);
    }

    void ModifyOrderPrice(const std::string& id, Money price,
        const boost::posix_time::ptime& timestamp) override {
      get_override("modify_order_price")(id, price, timestamp);
    }

    void DeleteOrder(const std::string& id,
        const boost::posix_time::ptime& timestamp) override {
      get_override("delete_order")(id, timestamp);
    }

    void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale) override {
      get_override("publish_time_and_sale")(timeAndSale);
    }

    void Open() override {
      get_override("open")();
    }

    void Close() override {
      get_override("close")();
    }
  };

  auto BuildFeedClient(CountryCode country, time_duration samplingTime,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    auto marketDataService = FindMarketDataFeedService(country,
      serviceLocatorClient);
    auto addresses = FromString<vector<IpAddress>>(
      get<string>(marketDataService->GetProperties().At("addresses")));
    auto marketDataFeedClient = std::make_unique<FeedClient>(
      Initialize(addresses, Ref(*GetSocketThreadPool())),
      SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(serviceLocatorClient)),
      Initialize(samplingTime, Ref(*GetTimerThreadPool())),
      Initialize(seconds(10), Ref(*GetTimerThreadPool())));
    return MakeToPythonMarketDataFeedClient(
      std::move(marketDataFeedClient)).release();
  }

  auto BuildMarketDataServiceTestEnvironment(
      const std::shared_ptr<VirtualServiceLocatorClient>& serviceLocatorClient,
      const std::shared_ptr<VirtualAdministrationClient>&
      administrationClient) {
    return new MarketDataServiceTestEnvironment{serviceLocatorClient,
      administrationClient};
  }

  std::unique_ptr<VirtualMarketDataClient>
      MarketDataServiceTestEnvironmentBuildClient(
      MarketDataServiceTestEnvironment& environment,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    return MakeToPythonMarketDataClient(
      environment.BuildClient(Ref(serviceLocatorClient)));
  }

  auto BuildMySqlHistoricalDataStore(
      std::string host, unsigned int port, std::string username,
      std::string password, std::string database) {
    return MakeToPythonHistoricalDataStore(
      std::make_unique<SqlHistoricalDataStore<Viper::MySql::Connection>>(
      [=] {
        return Viper::MySql::Connection(host, port, username, password,
          database);
      })).release();
  }

  auto BuildSqliteHistoricalDataStore(std::string path) {
    return std::shared_ptr(MakeToPythonHistoricalDataStore(
      std::make_unique<SqlHistoricalDataStore<Viper::Sqlite3::Connection>>(
      [=] {
        return Viper::Sqlite3::Connection(path);
      })));
  }
}

void Nexus::Python::ExportApplicationMarketDataClient() {
  class_<ToPythonMarketDataClient<Client>, bases<VirtualMarketDataClient>,
    boost::noncopyable>("ApplicationMarketDataClient", no_init)
    .def("__init__", make_constructor(&BuildClient));
}

void Nexus::Python::ExportApplicationMarketDataFeedClient() {
  class_<ToPythonMarketDataFeedClient<FeedClient>,
    bases<VirtualMarketDataFeedClient>, boost::noncopyable>(
    "ApplicationMarketDataFeedClient", no_init)
    .def("__init__", make_constructor(&BuildFeedClient));
}

void Nexus::Python::ExportHistoricalDataStore() {
  class_<FromPythonHistoricalDataStore, boost::noncopyable>(
    "HistoricalDataStore", no_init)
    .def("load_order_imbalances",
      pure_virtual(&VirtualHistoricalDataStore::LoadOrderImbalances))
    .def("load_bbo_quotes",
      pure_virtual(&VirtualHistoricalDataStore::LoadBboQuotes))
    .def("load_book_quotes",
      pure_virtual(&VirtualHistoricalDataStore::LoadBookQuotes))
    .def("load_market_quotes",
      pure_virtual(&VirtualHistoricalDataStore::LoadMarketQuotes))
    .def("load_time_and_sales",
      pure_virtual(&VirtualHistoricalDataStore::LoadTimeAndSales))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedMarketOrderImbalance&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedMarketOrderImbalance>&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBboQuote&)>(&VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBboQuote>&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityMarketQuote&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityMarketQuote>&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityBookQuote&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityBookQuote>&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const SequencedSecurityTimeAndSale&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("store", pure_virtual(static_cast<void (VirtualHistoricalDataStore::*)(
      const std::vector<SequencedSecurityTimeAndSale>&)>(
      &VirtualHistoricalDataStore::Store)))
    .def("open", pure_virtual(&VirtualHistoricalDataStore::Open))
    .def("close", pure_virtual(&VirtualHistoricalDataStore::Close));
  ExportUniquePtr<VirtualHistoricalDataStore>();
}

void Nexus::Python::ExportMarketDataClient() {
  class_<FromPythonMarketDataClient, boost::noncopyable>("MarketDataClient",
    no_init)
    .def("query_sequenced_order_imbalances", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(const MarketWideDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedOrderImbalance>>&)>(
      &VirtualMarketDataClient::QueryOrderImbalances)))
    .def("query_order_imbalances", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(const MarketWideDataQuery&,
      const std::shared_ptr<QueueWriter<OrderImbalance>>&)>(
      &VirtualMarketDataClient::QueryOrderImbalances)))
    .def("query_sequenced_bbo_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedBboQuote>>&)>(
      &VirtualMarketDataClient::QueryBboQuotes)))
    .def("query_bbo_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<BboQuote>>&)>(
      &VirtualMarketDataClient::QueryBboQuotes)))
    .def("query_sequenced_book_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedBookQuote>>&)>(
      &VirtualMarketDataClient::QueryBookQuotes)))
    .def("query_book_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<BookQuote>>&)>(
      &VirtualMarketDataClient::QueryBookQuotes)))
    .def("query_sequenced_market_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedMarketQuote>>&)>(
      &VirtualMarketDataClient::QueryMarketQuotes)))
    .def("query_market_quotes", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<MarketQuote>>&)>(
      &VirtualMarketDataClient::QueryMarketQuotes)))
    .def("query_sequenced_time_and_sales", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<SequencedTimeAndSale>>&)>(
      &VirtualMarketDataClient::QueryTimeAndSales)))
    .def("query_time_and_sales", pure_virtual(
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&,
      const std::shared_ptr<QueueWriter<TimeAndSale>>&)>(
      &VirtualMarketDataClient::QueryTimeAndSales)))
    .def("load_security_snapshot", pure_virtual(
      &VirtualMarketDataClient::LoadSecuritySnapshot))
    .def("load_security_technicals", pure_virtual(
      &VirtualMarketDataClient::LoadSecurityTechnicals))
    .def("load_security_info_from_prefix", pure_virtual(
      &VirtualMarketDataClient::LoadSecurityInfoFromPrefix))
    .def("open", pure_virtual(&VirtualMarketDataClient::Open))
    .def("close", pure_virtual(&VirtualMarketDataClient::Close));
  ExportUniquePtr<VirtualMarketDataClient>();
  ExportVector<vector<SecurityInfo>>("VectorSecurityInfo");
}

void Nexus::Python::ExportMarketDataFeedClient() {
  class_<FromPythonMarketDataFeedClient, boost::noncopyable>(
    "MarketDataFeedClient", no_init)
    .def("add", pure_virtual(&VirtualMarketDataFeedClient::Add))
    .def("publish_order_imbalance",
      pure_virtual(&VirtualMarketDataFeedClient::PublishOrderImbalance))
    .def("publish_bbo_quote",
      pure_virtual(&VirtualMarketDataFeedClient::PublishBboQuote))
    .def("publish_market_quote",
      pure_virtual(&VirtualMarketDataFeedClient::PublishMarketQuote))
    .def("set_book_quote",
      pure_virtual(&VirtualMarketDataFeedClient::SetBookQuote))
    .def("add_order", pure_virtual(&VirtualMarketDataFeedClient::AddOrder))
    .def("modify_order_size",
      pure_virtual(&VirtualMarketDataFeedClient::ModifyOrderSize))
    .def("offset_order_size",
      pure_virtual(&VirtualMarketDataFeedClient::OffsetOrderSize))
    .def("modify_order_price",
      pure_virtual(&VirtualMarketDataFeedClient::ModifyOrderPrice))
    .def("delete_order",
      pure_virtual(&VirtualMarketDataFeedClient::DeleteOrder))
    .def("publish_time_and_sale",
      pure_virtual(&VirtualMarketDataFeedClient::PublishTimeAndSale))
    .def("open", pure_virtual(&VirtualMarketDataFeedClient::Open))
    .def("close", pure_virtual(&VirtualMarketDataFeedClient::Close));
  ExportUniquePtr<VirtualMarketDataFeedClient>();
}

void Nexus::Python::ExportMarketDataService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".market_data_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("market_data_service") = nestedModule;
  scope parent = nestedModule;
  ExportHistoricalDataStore();
  ExportMarketDataClient();
  ExportApplicationMarketDataClient();
  ExportMarketDataFeedClient();
  ExportApplicationMarketDataFeedClient();
  ExportBasicQuery<MarketCode>("MarketWideData");
  ExportBasicQuery<Security>("SecurityMarketData");
  ExportSecuritySnapshot();
  ExportMySqlHistoricalDataStore();
  ExportSqliteHistoricalDataStore();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportMarketDataServiceTestEnvironment();
  }
}

void Nexus::Python::ExportMarketDataServiceTestEnvironment() {
  class_<MarketDataServiceTestEnvironment, boost::noncopyable>(
      "MarketDataServiceTestEnvironment", no_init)
    .def("__init__", make_constructor(BuildMarketDataServiceTestEnvironment))
    .def("open", BlockingFunction(&MarketDataServiceTestEnvironment::Open))
    .def("close", BlockingFunction(&MarketDataServiceTestEnvironment::Close))
    .def("publish", BlockingFunction(
      static_cast<void (MarketDataServiceTestEnvironment::*)(
      MarketCode, const OrderImbalance&)>(
      &MarketDataServiceTestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (MarketDataServiceTestEnvironment::*)(
      const Security&, const BboQuote&)>(
      &MarketDataServiceTestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (MarketDataServiceTestEnvironment::*)(const Security&,
      const BookQuote&)>(&MarketDataServiceTestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (MarketDataServiceTestEnvironment::*)(const Security&,
      const MarketQuote&)>(&MarketDataServiceTestEnvironment::Publish)))
    .def("publish", BlockingFunction(
      static_cast<void (MarketDataServiceTestEnvironment::*)(const Security&,
      const TimeAndSale&)>(&MarketDataServiceTestEnvironment::Publish)))
    .def("build_client", &MarketDataServiceTestEnvironmentBuildClient);
}

void Nexus::Python::ExportMySqlHistoricalDataStore() {
  class_<ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<Viper::MySql::Connection>>,
    bases<VirtualHistoricalDataStore>, boost::noncopyable>(
    "MySqlHistoricalDataStore", no_init)
    .def("__init__", make_constructor(&BuildMySqlHistoricalDataStore));
}

void Nexus::Python::ExportSecuritySnapshot() {
  class_<SecuritySnapshot>("SecuritySnapshot", init<>())
    .def(init<const Security&>())
    .def("__copy__", &MakeCopy<SecuritySnapshot>)
    .def("__deepcopy__", &MakeDeepCopy<SecuritySnapshot>)
    .def_readwrite("security", &SecuritySnapshot::m_security)
    .def_readwrite("bbo_quote", &SecuritySnapshot::m_bboQuote)
    .def_readwrite("time_and_sale", &SecuritySnapshot::m_timeAndSale)
    .def_readwrite("market_quotes", &SecuritySnapshot::m_marketQuotes)
    .def_readwrite("ask_book", &SecuritySnapshot::m_askBook)
    .def_readwrite("bid_book", &SecuritySnapshot::m_bidBook);
}

void Nexus::Python::ExportSqliteHistoricalDataStore() {
  using PythonSqliteHistoricalDataStore = ToPythonHistoricalDataStore<
    SqlHistoricalDataStore<Viper::Sqlite3::Connection>>;
  class_<PythonSqliteHistoricalDataStore,
    std::shared_ptr<PythonSqliteHistoricalDataStore>,
    bases<VirtualHistoricalDataStore>, boost::noncopyable>(
    "SqliteHistoricalDataStore", no_init)
    .def("__init__", make_constructor(&BuildSqliteHistoricalDataStore));
  implicitly_convertible<std::shared_ptr<PythonSqliteHistoricalDataStore>,
    std::shared_ptr<VirtualHistoricalDataStore>>();
}
