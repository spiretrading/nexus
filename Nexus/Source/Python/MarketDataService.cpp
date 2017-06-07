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
#include <Beam/Python/PythonQueueWriter.hpp>
#include <Beam/Python/Queries.hpp>
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
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/MarketDataServiceTests/MarketDataServiceTestEnvironment.hpp"
#include "Nexus/Python/PythonMarketDataClient.hpp"

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

  PythonMarketDataClient* BuildClient(
      VirtualServiceLocatorClient& serviceLocatorClient) {
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
    auto baseClient = std::make_unique<Client>(sessionBuilder);
    return new PythonMarketDataClient{
      MakeVirtualMarketDataClient(std::move(baseClient))};
  }

  MarketDataServiceTestEnvironment* BuildMarketDataServiceTestEnvironment(
      const std::shared_ptr<VirtualServiceLocatorClient>& serviceLocatorClient,
      const std::shared_ptr<VirtualAdministrationClient>&
      administrationClient) {
    return new MarketDataServiceTestEnvironment{serviceLocatorClient,
      administrationClient};
  }

  PythonMarketDataClient* MarketDataServiceTestEnvironmentBuildClient(
      MarketDataServiceTestEnvironment& environment,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    return new PythonMarketDataClient{
      environment.BuildClient(Ref(serviceLocatorClient))};
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile PythonMarketDataClient*
      get_pointer(const volatile PythonMarketDataClient* p) {
    return p;
  }
}
#endif

void Nexus::Python::ExportMarketDataClient() {
  class_<VirtualMarketDataClient, boost::noncopyable>("VirtualMarketDataClient",
    no_init);
  class_<PythonMarketDataClient, boost::noncopyable,
      bases<VirtualMarketDataClient>>("MarketDataClient", no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("query_order_imbalances",
      &PythonMarketDataClient::QueryOrderImbalances)
    .def("query_sequenced_order_imbalances",
      &PythonMarketDataClient::QuerySequencedOrderImbalances)
    .def("query_bbo_quotes", &PythonMarketDataClient::QueryBboQuotes)
    .def("query_sequenced_bbo_quotes",
      &PythonMarketDataClient::QuerySequencedBboQuotes)
    .def("query_book_quotes", &PythonMarketDataClient::QueryBookQuotes)
    .def("query_sequenced_book_quotes",
      &PythonMarketDataClient::QuerySequencedBookQuotes)
    .def("query_market_quotes", &PythonMarketDataClient::QueryMarketQuotes)
    .def("query_sequenced_market_quotes",
      &PythonMarketDataClient::QuerySequencedMarketQuotes)
    .def("query_time_and_sales", &PythonMarketDataClient::QueryTimeAndSales)
    .def("query_sequenced_time_and_sales",
      &PythonMarketDataClient::QuerySequencedTimeAndSales)
    .def("load_security_snapshot", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecuritySnapshot))
    .def("load_security_technicals", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecurityTechnicals))
    .def("load_security_info_from_prefix",
      BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::LoadSecurityInfoFromPrefix))
    .def("open", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::Open))
    .def("close", BlockingFunction<PythonMarketDataClient>(
      &PythonMarketDataClient::Close));
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
    .def("build_client", &MarketDataServiceTestEnvironmentBuildClient,
      return_value_policy<manage_new_object>());
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
