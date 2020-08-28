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
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/Reactors.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/HistoricalDataStore.hpp"
#include "Nexus/Python/MarketDataClient.hpp"
#include "Nexus/Python/MarketDataFeedClient.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
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
    boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(boost::optional<SecurityInfo>,
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
        ScopedQueueWriter<SequencedOrderImbalance> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_order_imbalances", QueryOrderImbalances, query,
        std::move(queue));
    }

    void QueryOrderImbalances(const MarketWideDataQuery& query,
        ScopedQueueWriter<OrderImbalance> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_order_imbalances", QueryOrderImbalances, query,
        std::move(queue));
    }

    void QueryBboQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<SequencedBboQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_bbo_quotes", QueryBboQuotes, query, std::move(queue));
    }

    void QueryBboQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<BboQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_bbo_quotes", QueryBboQuotes, query, std::move(queue));
    }

    void QueryBookQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<SequencedBookQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_book_quotes", QueryBookQuotes, query,
        std::move(queue));
    }

    void QueryBookQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<BookQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_book_quotes", QueryBookQuotes, query, std::move(queue));
    }

    void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<SequencedMarketQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_market_quotes", QueryMarketQuotes, query,
        std::move(queue));
    }

    void QueryMarketQuotes(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<MarketQuote> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_market_quotes", QueryMarketQuotes, query, std::move(queue));
    }

    void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<SequencedTimeAndSale> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_sequenced_time_and_sales", QueryTimeAndSales, query,
        std::move(queue));
    }

    void QueryTimeAndSales(const SecurityMarketDataQuery& query,
        ScopedQueueWriter<TimeAndSale> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataClient,
        "query_time_and_sales", QueryTimeAndSales, query, std::move(queue));
    }

    SecuritySnapshot LoadSecuritySnapshot(const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(SecuritySnapshot, VirtualMarketDataClient,
        "load_security_snapshot", LoadSecuritySnapshot, security);
    }

    SecurityTechnicals LoadSecurityTechnicals(
        const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(SecurityTechnicals, VirtualMarketDataClient,
        "load_security_technicals", LoadSecurityTechnicals, security);
    }

    boost::optional<SecurityInfo> LoadSecurityInfo(
        const Security& security) override {
      PYBIND11_OVERLOAD_PURE_NAME(boost::optional<SecurityInfo>,
        VirtualMarketDataClient, "load_security_info", LoadSecurityInfo,
        security);
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

  struct TrampolineMarketDataFeedClient final : VirtualMarketDataFeedClient {
    void Add(const SecurityInfo& securityInfo) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient, "add",
        Add, securityInfo);
    }

    void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance)
        override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "publish_order_imbalance", PublishOrderImbalance, orderImbalance);
    }

    void PublishBboQuote(const SecurityBboQuote& bboQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "publish_bbo_quote", PublishBboQuote, bboQuote);
    }

    void PublishMarketQuote(const SecurityMarketQuote& marketQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "publish_market_quote", PublishMarketQuote, marketQuote);
    }

    void SetBookQuote(const SecurityBookQuote& bookQuote) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "set_book_quote", SetBookQuote, bookQuote);
    }

    void AddOrder(const Security& security, MarketCode market,
        const std::string& mpid, bool isPrimaryMpid, const std::string& id,
        Side side, Money price, Quantity size,
        const ptime& timestamp) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "add_order", AddOrder, security, market, mpid, isPrimaryMpid, id, side,
        price, size, timestamp);
    }

    void ModifyOrderSize(const std::string& id, Quantity size,
        const ptime& timestamp) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "modify_order_size", ModifyOrderSize, id, size, timestamp);
    }

    void OffsetOrderSize(const std::string& id, Quantity delta,
        const ptime& timestamp) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "offset_order_size", OffsetOrderSize, id, delta, timestamp);
    }

    void ModifyOrderPrice(const std::string& id, Money price,
        const ptime& timestamp) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "modify_order_price", ModifyOrderPrice, id, price, timestamp);
    }

    void DeleteOrder(const std::string& id, const ptime& timestamp) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "delete_order", DeleteOrder, id, timestamp);
    }

    void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient,
        "publish_time_and_sale", PublishTimeAndSale, timeAndSale);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient, "open",
        Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualMarketDataFeedClient, "close",
        Close);
    }
  };

  using PythonMarketDataFeedClient = MarketDataFeedClient<std::string,
    LiveTimer, MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;

  auto MakePythonMarketDataFeedClient(
      VirtualServiceLocatorClient& serviceLocatorClient,
      time_duration sampling) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      MarketDataService::FEED_SERVICE_NAME);
    return MakeToPythonMarketDataFeedClient(
      std::make_unique<PythonMarketDataFeedClient>(
      Initialize(addresses, Ref(*GetSocketThreadPool())),
      SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(serviceLocatorClient)),
      Initialize(sampling, Ref(*GetTimerThreadPool())),
      Initialize(seconds(10), Ref(*GetTimerThreadPool()))));
  }

  auto MakePythonMarketDataFeedClient(
      VirtualServiceLocatorClient& serviceLocatorClient, CountryCode country,
      time_duration sampling) {
    auto service = FindMarketDataFeedService(country, serviceLocatorClient);
    if(!service.is_initialized()) {
      return MakePythonMarketDataFeedClient(serviceLocatorClient, sampling);
    }
    auto addresses = Parse<std::vector<IpAddress>>(get<std::string>(
      service->GetProperties().At("addresses")));
    return MakeToPythonMarketDataFeedClient(
      std::make_unique<PythonMarketDataFeedClient>(
      Initialize(addresses, Ref(*GetSocketThreadPool())),
      SessionAuthenticator<VirtualServiceLocatorClient>(
      Ref(serviceLocatorClient)),
      Initialize(sampling, Ref(*GetTimerThreadPool())),
      Initialize(seconds(10), Ref(*GetTimerThreadPool()))));
  }
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
          [] {
            return std::make_unique<LiveTimer>(seconds(10),
              Ref(*GetTimerThreadPool()));
          });
        return MakeToPythonMarketDataClient(std::make_unique<Client>(
          sessionBuilder));
      }));
}

void Nexus::Python::ExportApplicationMarketDataFeedClient(
    pybind11::module& module) {
  using Client = MarketDataFeedClient<std::string, LiveTimer,
    MessageProtocol<TcpSocketChannel, BinarySender<SharedBuffer>,
    SizeDeclarativeEncoder<ZLibEncoder>>, LiveTimer>;
  class_<ToPythonMarketDataFeedClient<Client>, VirtualMarketDataFeedClient>(
      module, "ApplicationMarketDataFeedClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient,
          CountryCode country, time_duration sampling) {
        return MakePythonMarketDataFeedClient(serviceLocatorClient, country,
          sampling);
      }))
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient,
          CountryCode country) {
        return MakePythonMarketDataFeedClient(serviceLocatorClient, country,
          milliseconds(10));
      }))
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient,
          time_duration sampling) {
        return MakePythonMarketDataFeedClient(serviceLocatorClient, sampling);
      }))
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        return MakePythonMarketDataFeedClient(serviceLocatorClient,
          milliseconds(10));
      }));
}

void Nexus::Python::ExportHistoricalDataStore(pybind11::module& module) {
  class_<VirtualHistoricalDataStore, TrampolineHistoricalDataStore,
      std::shared_ptr<VirtualHistoricalDataStore>>(module,
    "HistoricalDataStore")
    .def("load_security_info", &VirtualHistoricalDataStore::LoadSecurityInfo)
    .def("load_all_security_info",
      &VirtualHistoricalDataStore::LoadAllSecurityInfo)
    .def("load_order_imbalances",
      &VirtualHistoricalDataStore::LoadOrderImbalances)
    .def("load_bbo_quotes", &VirtualHistoricalDataStore::LoadBboQuotes)
    .def("load_book_quotes", &VirtualHistoricalDataStore::LoadBookQuotes)
    .def("load_market_quotes", &VirtualHistoricalDataStore::LoadMarketQuotes)
    .def("load_time_and_sales", &VirtualHistoricalDataStore::LoadTimeAndSales)
    .def("store", static_cast<void (VirtualHistoricalDataStore::*)(
      const SecurityInfo&)>(&VirtualHistoricalDataStore::Store))
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
      ScopedQueueWriter<SequencedOrderImbalance>)>(
      &VirtualMarketDataClient::QueryOrderImbalances))
    .def("query_order_imbalances",
      static_cast<void (VirtualMarketDataClient::*)(const MarketWideDataQuery&,
      ScopedQueueWriter<OrderImbalance>)>(
      &VirtualMarketDataClient::QueryOrderImbalances))
    .def("query_sequenced_bbo_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<SequencedBboQuote>)>(
      &VirtualMarketDataClient::QueryBboQuotes))
    .def("query_bbo_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<BboQuote>)>(
      &VirtualMarketDataClient::QueryBboQuotes))
    .def("query_sequenced_book_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<SequencedBookQuote>)>(
      &VirtualMarketDataClient::QueryBookQuotes))
    .def("query_book_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<BookQuote>)>(
      &VirtualMarketDataClient::QueryBookQuotes))
    .def("query_sequenced_market_quotes",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<SequencedMarketQuote>)>(
      &VirtualMarketDataClient::QueryMarketQuotes))
    .def("query_market_quotes", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<MarketQuote>)>(
      &VirtualMarketDataClient::QueryMarketQuotes))
    .def("query_sequenced_time_and_sales",
      static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<SequencedTimeAndSale>)>(
      &VirtualMarketDataClient::QueryTimeAndSales))
    .def("query_time_and_sales", static_cast<void (VirtualMarketDataClient::*)(
      const SecurityMarketDataQuery&, ScopedQueueWriter<TimeAndSale>)>(
      &VirtualMarketDataClient::QueryTimeAndSales))
    .def("load_security_snapshot",
      &VirtualMarketDataClient::LoadSecuritySnapshot)
    .def("load_security_technicals",
      &VirtualMarketDataClient::LoadSecurityTechnicals)
    .def("load_security_info", &VirtualMarketDataClient::LoadSecurityInfo)
    .def("load_security_info_from_prefix",
      &VirtualMarketDataClient::LoadSecurityInfoFromPrefix)
    .def("open", &VirtualMarketDataClient::Open)
    .def("close", &VirtualMarketDataClient::Close);
}

void Nexus::Python::ExportMarketDataFeedClient(pybind11::module& module) {
  class_<VirtualMarketDataFeedClient, TrampolineMarketDataFeedClient>(module,
      "MarketDataFeedClient")
    .def("add", &VirtualMarketDataFeedClient::Add)
    .def("publish_order_imbalance",
      &VirtualMarketDataFeedClient::PublishOrderImbalance)
    .def("publish_bbo_quote", &VirtualMarketDataFeedClient::PublishBboQuote)
    .def("publish_market_quote",
      &VirtualMarketDataFeedClient::PublishMarketQuote)
    .def("set_book_quote", &VirtualMarketDataFeedClient::SetBookQuote)
    .def("add_order", &VirtualMarketDataFeedClient::AddOrder)
    .def("modify_order_size", &VirtualMarketDataFeedClient::ModifyOrderSize)
    .def("offset_order_size", &VirtualMarketDataFeedClient::OffsetOrderSize)
    .def("modify_order_price", &VirtualMarketDataFeedClient::ModifyOrderPrice)
    .def("delete_order", &VirtualMarketDataFeedClient::DeleteOrder)
    .def("publish_time_and_sale",
      &VirtualMarketDataFeedClient::PublishTimeAndSale)
    .def("open", &VirtualMarketDataFeedClient::Open)
    .def("close", &VirtualMarketDataFeedClient::Close);
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
  ExportMarketDataFeedClient(submodule);
  ExportApplicationMarketDataClient(submodule);
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
