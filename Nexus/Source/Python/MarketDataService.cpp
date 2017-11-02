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
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/ToPythonMarketDataClient.hpp"

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
    SessionBuilder sessionBuilder(Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          LiveTimer delayTimer(seconds(3), Ref(*GetTimerThreadPool()));
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
}

void Nexus::Python::ExportApplicationMarketDataClient() {
  class_<ToPythonMarketDataClient<Client>, bases<VirtualMarketDataClient>,
    boost::noncopyable>("ApplicationMarketDataClient", no_init)
    .def("__init__", make_constructor(&BuildClient));
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

void Nexus::Python::ExportMarketDataService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".market_data_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("market_data_service") = nestedModule;
  scope parent = nestedModule;
  ExportMarketDataClient();
  ExportApplicationMarketDataClient();
  ExportMarketWideDataQuery();
  ExportSecurityMarketDataQuery();
  ExportSecuritySnapshot();
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

void Nexus::Python::ExportMarketWideDataQuery() {
  ExportIndexedQuery<MarketCode>("MarketWideDataQuery");
  class_<MarketWideDataQuery, bases<IndexedQuery<MarketCode>, RangedQuery,
    SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>>(
    "MarketWideDataQuery", init<>())
    .def("__copy__", &MakeCopy<MarketWideDataQuery>)
    .def("__deepcopy__", &MakeDeepCopy<MarketWideDataQuery>);
  def("build_real_time_with_snapshot_query",
    static_cast<MarketWideDataQuery (*)(const MarketCode&)>(
    &BuildRealTimeWithSnapshotQuery));
}

void Nexus::Python::ExportSecurityMarketDataQuery() {
  ExportIndexedQuery<Security>("SecurityIndexedQuery");
  class_<SecurityMarketDataQuery, bases<IndexedQuery<Security>, RangedQuery,
    SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>>(
    "SecurityMarketDataQuery", init<>())
    .def("__copy__", &MakeCopy<SecurityMarketDataQuery>)
    .def("__deepcopy__", &MakeDeepCopy<SecurityMarketDataQuery>);
  def("build_real_time_with_snapshot_query",
    static_cast<SecurityMarketDataQuery (*)(Security)>(
    &BuildRealTimeWithSnapshotQuery));
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
  ExportVector<vector<SequencedBookQuote>>("VectorSequencedBookQuote");
}
