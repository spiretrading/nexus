#include "Nexus/Python/OrderExecutionService.hpp"
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Python/BoostPython.hpp>
#include <Beam/Python/GilRelease.hpp>
#include <Beam/Python/ListToVector.hpp>
#include <Beam/Python/PythonBindings.hpp>
#include <Beam/Python/Queries.hpp>
#include <Beam/Python/Queues.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ServiceLocatorClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Services/ServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceInstance.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/Python/PythonOrderExecutionClient.hpp"

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
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Python;
using namespace std;

namespace {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = OrderExecutionClient<SessionBuilder>;

  PythonOrderExecutionClient* BuildClient(
      VirtualServiceLocatorClient& serviceLocatorClient) {
    auto addresses = LocateServiceAddresses(serviceLocatorClient,
      OrderExecutionService::SERVICE_NAME);
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
    return new PythonOrderExecutionClient{
      MakeVirtualOrderExecutionClient(std::move(baseClient))};
  }

  void PythonQueryDailyOrderSubmissions(const DirectoryEntry& account,
      const ptime& startTime, const ptime& endTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase,
      PythonOrderExecutionClient* orderExecutionClient,
      const std::shared_ptr<PythonQueueWriter>& queue) {
    QueryDailyOrderSubmissions(account, startTime, endTime, marketDatabase,
      timeZoneDatabase, static_cast<VirtualOrderExecutionClient&>(
      *orderExecutionClient), queue->GetSlot<const Order*>());
  }

  OrderExecutionServiceTestInstance* BuildOrderExecutionServiceTestInstance(
      std::auto_ptr<VirtualServiceLocatorClient> serviceLocatorClient,
      std::auto_ptr<VirtualUidClient> uidClient,
      std::auto_ptr<VirtualAdministrationClient> administrationClient) {
    std::shared_ptr<VirtualServiceLocatorClient> serviceLocatorClientWrapper{
      serviceLocatorClient.release(), [] (VirtualServiceLocatorClient*) {}};
    std::unique_ptr<VirtualUidClient> uidClientWrapper{uidClient.release()};
    std::unique_ptr<VirtualAdministrationClient> administrationClientWrapper{
      administrationClient.release()};
    return new OrderExecutionServiceTestInstance{serviceLocatorClientWrapper,
      std::move(uidClientWrapper), std::move(administrationClientWrapper)};
  }

  PythonOrderExecutionClient* OrderExecutionServiceTestInstanceBuildClient(
      OrderExecutionServiceTestInstance& instance,
      VirtualServiceLocatorClient& serviceLocatorClient) {
    return new PythonOrderExecutionClient{
      instance.BuildClient(Ref(serviceLocatorClient))};
  }
}

void Nexus::Python::ExportAccountQuery() {
  ExportIndexedQuery<DirectoryEntry>("DirectoryEntryIndexedQuery");
  class_<AccountQuery, bases<IndexedQuery<DirectoryEntry>, RangedQuery,
    SnapshotLimitedQuery, InterruptableQuery, FilteredQuery>>(
    "AccountQuery", init<>());
}

void Nexus::Python::ExportExecutionReport() {
  ExportPublisher<ExecutionReport>("ExecutionReportPublisher");
  ExportSnapshotPublisher<ExecutionReport, vector<ExecutionReport>>(
    "ExecutionReportSnapshotPublisher");
  class_<ExecutionReport>("ExecutionReport", init<>())
    .def("build_initial_report", &ExecutionReport::BuildInitialReport)
    .staticmethod("build_initial_report")
    .def("build_updated_report", &ExecutionReport::BuildUpdatedReport)
    .staticmethod("build_updated_report")
    .def_readwrite("id", &ExecutionReport::m_id)
    .add_property("timestamp", make_getter(&ExecutionReport::m_timestamp,
      return_value_policy<return_by_value>()), make_setter(
      &ExecutionReport::m_timestamp, return_value_policy<return_by_value>()))
    .def_readwrite("sequence", &ExecutionReport::m_sequence)
    .add_property("status", make_getter(&ExecutionReport::m_status,
      return_value_policy<return_by_value>()), make_setter(
      &ExecutionReport::m_status, return_value_policy<return_by_value>()))
    .def_readwrite("last_quantity", &ExecutionReport::m_lastQuantity)
    .def_readwrite("last_price", &ExecutionReport::m_lastPrice)
    .def_readwrite("liquidity_flag", &ExecutionReport::m_liquidityFlag)
    .def_readwrite("last_market", &ExecutionReport::m_lastMarket)
    .def_readwrite("execution_fee", &ExecutionReport::m_executionFee)
    .def_readwrite("processing_fee", &ExecutionReport::m_processingFee)
    .def_readwrite("commission", &ExecutionReport::m_commission)
    .def_readwrite("text", &ExecutionReport::m_text)
    .def_readwrite("additional_tags", &ExecutionReport::m_additionalTags)
    .def(self == self)
    .def(self != self);
  class_<vector<ExecutionReport>>("VectorExecutionReport")
    .def(vector_indexing_suite<vector<ExecutionReport>>());
  ExportVector<vector<ExecutionReport>>();
}

void Nexus::Python::ExportMockOrderExecutionDriver() {
  class_<MockOrderExecutionDriver, boost::noncopyable>(
      "MockOrderExecutionDriver", init<>())
    .def("set_order_status_new_on_submission",
      &MockOrderExecutionDriver::SetOrderStatusNewOnSubmission)
    .def("find_order", &MockOrderExecutionDriver::FindOrder,
      return_value_policy<reference_existing_object>())
    .def("add_recovery", &MockOrderExecutionDriver::AddRecovery)
    .def("get_publisher", &MockOrderExecutionDriver::GetPublisher,
      return_value_policy<reference_existing_object>())
    .def("recover", &MockOrderExecutionDriver::Recover,
      return_value_policy<reference_existing_object>())
    .def("submit", &MockOrderExecutionDriver::Submit,
      return_value_policy<reference_existing_object>())
    .def("cancel", &MockOrderExecutionDriver::Cancel)
    .def("update", &MockOrderExecutionDriver::Update)
    .def("open", BlockingFunction(&MockOrderExecutionDriver::Open))
    .def("close", BlockingFunction(&MockOrderExecutionDriver::Close));
}

void Nexus::Python::ExportOrder() {
  ExportPublisher<const Order*>("OrderPublisher");
  ExportSnapshotPublisher<const Order*, vector<const Order*>>(
    "OrderSnapshotPublisher");
  class_<Order, boost::noncopyable>("Order", no_init)
    .add_property("info", make_function(&Order::GetInfo,
      return_value_policy<copy_const_reference>()))
    .def("get_publisher", &Order::GetPublisher,
      return_value_policy<reference_existing_object>());
  class_<vector<const Order*>>("VectorOrder")
    .def(vector_indexing_suite<vector<const Order*>>());
}

void Nexus::Python::ExportOrderExecutionClient() {
  class_<VirtualOrderExecutionClient, boost::noncopyable>(
      "VirtualOrderExecutionClient", no_init);
  class_<PythonOrderExecutionClient, boost::noncopyable,
      bases<VirtualOrderExecutionClient>>("OrderExecutionClient", no_init)
    .def("__init__", make_constructor(&BuildClient))
    .def("query_order_records", &PythonOrderExecutionClient::QueryOrderRecords)
    .def("query_sequenced_order_submissions",
      &PythonOrderExecutionClient::QuerySequencedOrderSubmissions)
    .def("query_order_submissions",
      &PythonOrderExecutionClient::QueryOrderSubmissions)
    .def("query_execution_reports",
      &PythonOrderExecutionClient::QueryExecutionReports)
    .def("get_order_submission_publisher",
      &PythonOrderExecutionClient::GetOrderSubmissionPublisher,
      return_value_policy<reference_existing_object>())
    .def("submit", BlockingFunction<PythonOrderExecutionClient>(
      &PythonOrderExecutionClient::Submit,
      return_value_policy<reference_existing_object>()))
    .def("cancel", &PythonOrderExecutionClient::Cancel)
    .def("open", BlockingFunction<PythonOrderExecutionClient>(
      &PythonOrderExecutionClient::Open))
    .def("close", BlockingFunction<PythonOrderExecutionClient>(
      &PythonOrderExecutionClient::Close));
}

void Nexus::Python::ExportOrderExecutionService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".order_execution_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("order_execution_service") = nestedModule;
  scope parent = nestedModule;
  ExportAccountQuery();
  ExportExecutionReport();
  ExportOrder();
  ExportOrderExecutionClient();
  ExportOrderFields();
  ExportOrderInfo();
  ExportOrderRecord();
  ExportPrimitiveOrder();
  ExportStandardQueries();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportOrderExecutionServiceTestInstance();
    ExportMockOrderExecutionDriver();
    def("cancel_order", &CancelOrder);
    def("set_order_status", &SetOrderStatus);
    def("fill_order", static_cast<void (*)(PrimitiveOrder& order, Money price,
      Quantity quantity, const ptime& timestamp)>(&FillOrder));
    def("fill_order", static_cast<void (*)(PrimitiveOrder& order,
      Quantity quantity, const ptime& timestamp)>(&FillOrder));
  }
}

void Nexus::Python::ExportOrderExecutionServiceTestInstance() {
  class_<OrderExecutionServiceTestInstance, boost::noncopyable>(
      "OrderExecutionServiceTestInstance", no_init)
    .def("__init__", make_constructor(BuildOrderExecutionServiceTestInstance))
    .def("get_driver", &OrderExecutionServiceTestInstance::GetDriver,
      return_value_policy<reference_existing_object>())
    .def("open", BlockingFunction(&OrderExecutionServiceTestInstance::Open))
    .def("close", BlockingFunction(&OrderExecutionServiceTestInstance::Close))
    .def("build_client", &OrderExecutionServiceTestInstanceBuildClient,
      return_value_policy<manage_new_object>());
}

void Nexus::Python::ExportOrderFields() {
  class_<OrderFields>("OrderFields", init<>())
    .def("build_limit_order", &OrderFields::BuildLimitOrder)
    .staticmethod("build_limit_order")
    .def("build_market_order", &OrderFields::BuildMarketOrder)
    .staticmethod("build_market_order")
    .def_readwrite("account", &OrderFields::m_account)
    .def_readwrite("security", &OrderFields::m_security)
    .def_readwrite("currency", &OrderFields::m_currency)
    .add_property("type", make_getter(&OrderFields::m_type,
      return_value_policy<return_by_value>()), make_setter(&OrderFields::m_type,
      return_value_policy<return_by_value>()))
    .add_property("side", make_getter(&OrderFields::m_side,
      return_value_policy<return_by_value>()), make_setter(&OrderFields::m_side,
      return_value_policy<return_by_value>()))
    .def_readwrite("destination", &OrderFields::m_destination)
    .def_readwrite("quantity", &OrderFields::m_quantity)
    .def_readwrite("price", &OrderFields::m_price)
    .def_readwrite("time_in_force", &OrderFields::m_timeInForce)
    .def_readwrite("additional_fields", &OrderFields::m_additionalFields)
    .def(self < self)
    .def(self == self);
  def("find_field", &FindField);
  def("has_field", &HasField);
}

void Nexus::Python::ExportOrderInfo() {
  class_<OrderInfo>("OrderInfo", init<>())
    .def(init<OrderFields, DirectoryEntry, OrderId, bool, ptime>())
    .def(init<OrderFields, OrderId, bool, ptime>())
    .def(init<OrderFields, OrderId, ptime>())
    .def_readwrite("fields", &OrderInfo::m_fields)
    .def_readwrite("submission_account", &OrderInfo::m_submissionAccount)
    .def_readwrite("order_id", &OrderInfo::m_orderId)
    .def_readwrite("shorting_flag", &OrderInfo::m_shortingFlag)
    .add_property("timestamp", make_getter(&OrderInfo::m_timestamp,
      return_value_policy<return_by_value>()),
      make_setter(&OrderInfo::m_timestamp,
      return_value_policy<return_by_value>()))
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportOrderRecord() {
  class_<OrderRecord>("OrderRecord", init<>())
    .def(init<OrderInfo, vector<ExecutionReport>>())
    .def_readwrite("info", &OrderRecord::m_info)
    .def_readwrite("execution_reports", &OrderRecord::m_executionReports)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportPrimitiveOrder() {
  ExportPublisher<const PrimitiveOrder*>("ConstPrimitiveOrderPublisher");
  ExportPublisher<PrimitiveOrder*>("PrimitiveOrderPublisher");
  ExportSnapshotPublisher<const PrimitiveOrder*, vector<const PrimitiveOrder*>>(
    "PrimitiveOrderSnapshotPublisher");
  ExportSnapshotPublisher<PrimitiveOrder*, vector<PrimitiveOrder*>>(
    "ConstPrimitiveOrderSnapshotPublisher");
  class_<PrimitiveOrder, bases<Order>, boost::noncopyable>("PrimitiveOrder",
    init<OrderInfo>())
    .def(init<OrderRecord>())
    .def("update", &PrimitiveOrder::Update);
  def("build_rejected_order", &BuildRejectedOrder);
  def("reject_cancel_request", &RejectCancelRequest);
}

void Nexus::Python::ExportStandardQueries() {
  def("build_daily_order_submission_query", &BuildDailyOrderSubmissionQuery);
  def("query_daily_order_submissions", &PythonQueryDailyOrderSubmissions);
}
