#include "Nexus/Python/OrderExecutionService.hpp"
#include <Aspen/Python/Box.hpp>
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
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/OrderInfo.hpp"
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"
#include "Nexus/OrderExecutionService/OrderWrapperReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/Python/OrderExecutionClient.hpp"

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
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineOrderExecutionClient final : VirtualOrderExecutionClient {
    void QueryOrderRecords(const AccountQuery& query,
        const std::shared_ptr<QueueWriter<OrderRecord>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient,
        "query_order_records", QueryOrderRecords, query, queue);
    }

    void QueryOrderSubmissions(const AccountQuery& query,
        const std::shared_ptr<QueueWriter<SequencedOrder>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient,
        "query_sequenced_order_submissions", QueryOrderSubmissions, query,
        queue);
    }

    void QueryOrderSubmissions(const AccountQuery& query, const std::shared_ptr<
        QueueWriter<const Order*>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient,
        "query_order_submissions", QueryOrderSubmissions, query, queue);
    }

    void QueryExecutionReports(const AccountQuery& query, const std::shared_ptr<
        QueueWriter<ExecutionReport>>& queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient,
        "query_execution_reports", QueryExecutionReports, query, queue);
    }

    const Order& Submit(const OrderFields& fields) override {
      PYBIND11_OVERLOAD_PURE_NAME(const Order&, VirtualOrderExecutionClient,
        "submit", Submit, fields);
    }

    void Cancel(const Order& order) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient, "cancel",
        Cancel);
    }

    void Update(OrderId orderId,
        const ExecutionReport& executionReport) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient, "update",
        Update, orderId, executionReport);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient, "open",
        Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualOrderExecutionClient, "close",
        Close);
    }
  };
}

void Nexus::Python::ExportApplicationOrderExecutionClient(
    pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = OrderExecutionClient<SessionBuilder>;
  class_<ToPythonOrderExecutionClient<Client>, VirtualOrderExecutionClient>(
      module, "ApplicationOrderExecutionClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          OrderExecutionService::SERVICE_NAME);
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
        return MakeToPythonOrderExecutionClient(std::make_unique<Client>(
          sessionBuilder));
      }));
}

void Nexus::Python::ExportExecutionReport(pybind11::module& module) {
  class_<ExecutionReport>(module, "ExecutionReport")
    .def(init())
    .def(init<const ExecutionReport&>())
    .def_static("build_initial_report", &ExecutionReport::BuildInitialReport)
    .def_static("build_updated_report", &ExecutionReport::BuildUpdatedReport)
    .def_readwrite("id", &ExecutionReport::m_id)
    .def_readwrite("timestamp", &ExecutionReport::m_timestamp)
    .def_readwrite("sequence", &ExecutionReport::m_sequence)
    .def_readwrite("status", &ExecutionReport::m_status)
    .def_readwrite("last_quantity", &ExecutionReport::m_lastQuantity)
    .def_readwrite("last_price", &ExecutionReport::m_lastPrice)
    .def_readwrite("liquidity_flag", &ExecutionReport::m_liquidityFlag)
    .def_readwrite("last_market", &ExecutionReport::m_lastMarket)
    .def_readwrite("execution_fee", &ExecutionReport::m_executionFee)
    .def_readwrite("processing_fee", &ExecutionReport::m_processingFee)
    .def_readwrite("commission", &ExecutionReport::m_commission)
    .def_readwrite("text", &ExecutionReport::m_text)
    .def_readwrite("additional_tags", &ExecutionReport::m_additionalTags)
    .def("__str__", &lexical_cast<std::string, ExecutionReport>)
    .def(self == self)
    .def(self != self);
  ExportQueueSuite<ExecutionReport>(module, "ExecutionReport");
  ExportQueueSuite<SequencedExecutionReport>(module,
    "SequencedExecutionReport");
  ExportSnapshotPublisher<ExecutionReport, std::vector<ExecutionReport>>(module,
    "ExecutionReport");
}

void Nexus::Python::ExportMockOrderExecutionDriver(pybind11::module& module) {
  class_<MockOrderExecutionDriver>(module, "MockOrderExecutionDriver")
    .def(init())
    .def("set_order_status_new_on_submission",
      &MockOrderExecutionDriver::SetOrderStatusNewOnSubmission)
    .def("find_order", &MockOrderExecutionDriver::FindOrder,
      return_value_policy::reference_internal)
    .def("add_recovery", &MockOrderExecutionDriver::AddRecovery)
    .def("get_publisher", &MockOrderExecutionDriver::GetPublisher,
      return_value_policy::reference_internal)
    .def("recover", &MockOrderExecutionDriver::Recover,
      return_value_policy::reference_internal)
    .def("submit", &MockOrderExecutionDriver::Submit,
      call_guard<GilRelease>(), return_value_policy::reference_internal)
    .def("cancel", &MockOrderExecutionDriver::Cancel, call_guard<GilRelease>())
    .def("update", &MockOrderExecutionDriver::Update, call_guard<GilRelease>())
    .def("open", &MockOrderExecutionDriver::Open, call_guard<GilRelease>())
    .def("close", &MockOrderExecutionDriver::Close, call_guard<GilRelease>());
}

void Nexus::Python::ExportOrder(pybind11::module& module) {
  class_<Order>(module, "Order")
    .def_property_readonly("info", &Order::GetInfo)
    .def("get_publisher", &Order::GetPublisher,
      return_value_policy::reference_internal);
  ExportQueueSuite<const Order*>(module, "Order");
  ExportQueueSuite<SequencedOrder>(module, "SequencedOrder");
  ExportSnapshotPublisher<const Order*, std::vector<const Order*>>(module,
    "Order");
}

void Nexus::Python::ExportOrderCancellationReactor(pybind11::module& module) {
  auto aspenModule = pybind11::module::import("aspen");
  Aspen::export_box<const Order*>(aspenModule, "Order");
  Aspen::export_reactor<OrderCancellationReactor<VirtualOrderExecutionClient,
    Aspen::SharedBox<const Order*>>>(module, "OrderCancellationReactor")
    .def(init<Ref<VirtualOrderExecutionClient>,
    Aspen::SharedBox<const Order*>>());
}

void Nexus::Python::ExportOrderExecutionClient(pybind11::module& module) {
  class_<VirtualOrderExecutionClient, TrampolineOrderExecutionClient>(module,
    "OrderExecutionClient")
    .def("query_order_records", &VirtualOrderExecutionClient::QueryOrderRecords)
    .def("query_sequenced_order_submissions",
      static_cast<void (VirtualOrderExecutionClient::*)(const AccountQuery&,
      const std::shared_ptr<QueueWriter<SequencedOrder>>&)>(
      &VirtualOrderExecutionClient::QueryOrderSubmissions))
    .def("query_order_submissions",
      static_cast<void (VirtualOrderExecutionClient::*)(const AccountQuery&,
      const std::shared_ptr<QueueWriter<const Order*>>&)>(
      &VirtualOrderExecutionClient::QueryOrderSubmissions))
    .def("query_execution_reports",
      &VirtualOrderExecutionClient::QueryExecutionReports)
    .def("submit", &VirtualOrderExecutionClient::Submit,
      return_value_policy::reference_internal)
    .def("cancel", &VirtualOrderExecutionClient::Cancel)
    .def("update", &VirtualOrderExecutionClient::Update)
    .def("open", &VirtualOrderExecutionClient::Open)
    .def("close", &VirtualOrderExecutionClient::Close);
}

void Nexus::Python::ExportOrderExecutionService(pybind11::module& module) {
  auto submodule = module.def_submodule("order_execution_service");
  ExportExecutionReport(submodule);
  ExportOrder(submodule);
  ExportOrderExecutionClient(submodule);
  ExportApplicationOrderExecutionClient(submodule);
  ExportOrderFields(submodule);
  ExportOrderInfo(submodule);
  ExportOrderCancellationReactor(submodule);
  ExportOrderReactor(submodule);
  ExportOrderRecord(submodule);
  ExportPrimitiveOrder(submodule);
  ExportStandardQueries(submodule);
  ExportOrderWrapperReactor(submodule);
  auto testModule = submodule.def_submodule("tests");
  ExportOrderExecutionServiceTestEnvironment(testModule);
  ExportMockOrderExecutionDriver(testModule);
  testModule.def("cancel_order", &CancelOrder, call_guard<GilRelease>());
  testModule.def("set_order_status", &SetOrderStatus, call_guard<GilRelease>());
  testModule.def("fill_order",
    static_cast<void (*)(PrimitiveOrder& order, Money price,
    Quantity quantity, const ptime& timestamp)>(&FillOrder),
    call_guard<GilRelease>());
  testModule.def("fill_order",
    static_cast<void (*)(PrimitiveOrder& order,
    Quantity quantity, const ptime& timestamp)>(&FillOrder),
    call_guard<GilRelease>());
  testModule.def("is_pending_cancel", &IsPendingCancel,
    call_guard<GilRelease>());
}

void Nexus::Python::ExportOrderExecutionServiceTestEnvironment(
    pybind11::module& module) {
  class_<OrderExecutionServiceTestEnvironment>(module,
      "OrderExecutionServiceTestEnvironment")
    .def(init<const MarketDatabase&, const DestinationDatabase&,
      std::shared_ptr<VirtualServiceLocatorClient>,
      std::shared_ptr<VirtualUidClient>,
      std::shared_ptr<VirtualAdministrationClient>>())
    .def("get_driver", &OrderExecutionServiceTestEnvironment::GetDriver,
      return_value_policy::reference_internal)
    .def("open", &OrderExecutionServiceTestEnvironment::Open,
      call_guard<GilRelease>())
    .def("close", &OrderExecutionServiceTestEnvironment::Close,
      call_guard<GilRelease>())
    .def("build_client",
      [] (OrderExecutionServiceTestEnvironment& self,
          VirtualServiceLocatorClient& serviceLocatorClient) {
        return MakeToPythonOrderExecutionClient(
          self.BuildClient(Ref(serviceLocatorClient)));
      });
}

void Nexus::Python::ExportOrderFields(pybind11::module& module) {
  class_<OrderFields>(module, "OrderFields")
    .def(init())
    .def(init<const OrderFields&>())
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, CurrencyId, Side,
      const std::string&, Quantity, Money)>(&OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const Security&, CurrencyId, Side, const std::string&, Quantity, Money)>(
      &OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, Side, const std::string&,
      Quantity, Money)>(&OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const Security&, Side, const std::string&, Quantity, Money)>(
      &OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const Security&, CurrencyId, Side, Quantity, Money)>(
      &OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, Side, Quantity, Money)>(
      &OrderFields::BuildLimitOrder))
    .def_static("build_limit_order", static_cast<OrderFields (*)(
      const Security&, Side, Quantity, Money)>(&OrderFields::BuildLimitOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, CurrencyId, Side,
      const std::string&, Quantity)>(&OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const Security&, CurrencyId, Side, const std::string&, Quantity)>(
      &OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, Side, const std::string&,
      Quantity)>(&OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const Security&, Side, const std::string&, Quantity)>(
      &OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const Security&, CurrencyId, Side, Quantity)>(
      &OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const DirectoryEntry&, const Security&, Side, Quantity)>(
      &OrderFields::BuildMarketOrder))
    .def_static("build_market_order", static_cast<OrderFields (*)(
      const Security&, Side, Quantity)>(&OrderFields::BuildMarketOrder))
    .def_readwrite("account", &OrderFields::m_account)
    .def_readwrite("security", &OrderFields::m_security)
    .def_readwrite("currency", &OrderFields::m_currency)
    .def_readwrite("type", &OrderFields::m_type)
    .def_readwrite("side", &OrderFields::m_side)
    .def_readwrite("destination", &OrderFields::m_destination)
    .def_readwrite("quantity", &OrderFields::m_quantity)
    .def_readwrite("price", &OrderFields::m_price)
    .def_readwrite("time_in_force", &OrderFields::m_timeInForce)
    .def_readwrite("additional_fields", &OrderFields::m_additionalFields)
    .def("__str__", lexical_cast<std::string, OrderFields>)
    .def(self < self)
    .def(self == self);
  module.def("find_field", &FindField);
  module.def("has_field", &HasField);
}

void Nexus::Python::ExportOrderInfo(pybind11::module& module) {
  class_<OrderInfo>(module, "OrderInfo")
    .def(init())
    .def(init<OrderFields, DirectoryEntry, OrderId, bool, ptime>())
    .def(init<OrderFields, OrderId, bool, ptime>())
    .def(init<OrderFields, OrderId, ptime>())
    .def(init<const OrderInfo&>())
    .def_readwrite("fields", &OrderInfo::m_fields)
    .def_readwrite("submission_account", &OrderInfo::m_submissionAccount)
    .def_readwrite("order_id", &OrderInfo::m_orderId)
    .def_readwrite("shorting_flag", &OrderInfo::m_shortingFlag)
    .def_readwrite("timestamp", &OrderInfo::m_timestamp)
    .def("__str__", lexical_cast<std::string, OrderInfo>)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportOrderReactor(pybind11::module& module) {
  auto aspenModule = pybind11::module::import("aspen");
  Aspen::export_box<CurrencyId>(aspenModule, "CurrencyId");
  Aspen::export_box<OrderType>(aspenModule, "OrderType");
  Aspen::export_box<Side>(aspenModule, "Side");
  Aspen::export_box<Quantity>(aspenModule, "Quantity");
  Aspen::export_box<Money>(aspenModule, "Money");
  Aspen::export_box<TimeInForce>(aspenModule, "TimeInForce");
  Aspen::export_box<Tag>(aspenModule, "Tag");
  Aspen::export_reactor<OrderReactor<VirtualOrderExecutionClient,
    Aspen::SharedBox<DirectoryEntry>, Aspen::SharedBox<Security>,
    Aspen::SharedBox<CurrencyId>, Aspen::SharedBox<OrderType>,
    Aspen::SharedBox<Side>, Aspen::SharedBox<std::string>,
    Aspen::SharedBox<Quantity>, Aspen::SharedBox<Money>,
    Aspen::SharedBox<TimeInForce>, Aspen::SharedBox<Tag>>>(module,
    "OrderReactor")
    .def(init<Ref<VirtualOrderExecutionClient>,
      Aspen::SharedBox<DirectoryEntry>, Aspen::SharedBox<Security>,
      Aspen::SharedBox<CurrencyId>, Aspen::SharedBox<OrderType>,
      Aspen::SharedBox<Side>, Aspen::SharedBox<std::string>,
      Aspen::SharedBox<Quantity>, Aspen::SharedBox<Money>,
      Aspen::SharedBox<TimeInForce>, std::vector<Aspen::SharedBox<Tag>>>());
  module.def("make_limit_order_reactor",
    [] (VirtualOrderExecutionClient& client,
        Aspen::SharedBox<DirectoryEntry> account,
        Aspen::SharedBox<Security> security,
        Aspen::SharedBox<CurrencyId> currency, Aspen::SharedBox<Side> side,
        Aspen::SharedBox<std::string> destination,
        Aspen::SharedBox<Quantity> quantity, Aspen::SharedBox<Money> price,
        Aspen::SharedBox<TimeInForce> timeInForce) {
      return Aspen::to_object(MakeLimitOrderReactor(Ref(client),
        std::move(account), std::move(security), std::move(currency),
        std::move(side), std::move(destination), std::move(quantity),
        std::move(price), std::move(timeInForce)));
    });
  module.def("make_limit_order_reactor",
    [] (VirtualOrderExecutionClient& client,
        Aspen::SharedBox<DirectoryEntry> account,
        Aspen::SharedBox<Security> security,
        Aspen::SharedBox<CurrencyId> currency, Aspen::SharedBox<Side> side,
        Aspen::SharedBox<std::string> destination,
        Aspen::SharedBox<Quantity> quantity, Aspen::SharedBox<Money> price) {
      return Aspen::to_object(MakeLimitOrderReactor(Ref(client),
        std::move(account), std::move(security), std::move(currency),
        std::move(side), std::move(destination), std::move(quantity),
        std::move(price)));
    });
  module.def("make_limit_order_reactor",
    [] (VirtualOrderExecutionClient& client,
        Aspen::SharedBox<Security> security, Aspen::SharedBox<Side> side,
        Aspen::SharedBox<Quantity> quantity, Aspen::SharedBox<Money> price) {
      return Aspen::to_object(MakeLimitOrderReactor(Ref(client),
        std::move(security), std::move(side), std::move(quantity),
        std::move(price)));
    });
  module.def("make_limit_order_reactor",
    [] (VirtualOrderExecutionClient& client,
        Aspen::SharedBox<Security> security, Aspen::SharedBox<Side> side,
        Aspen::SharedBox<Quantity> quantity, Aspen::SharedBox<Money> price,
        Aspen::SharedBox<TimeInForce> timeInForce) {
      return Aspen::to_object(MakeLimitOrderReactor(Ref(client),
        std::move(security), std::move(side), std::move(quantity),
        std::move(price), std::move(timeInForce)));
    });
  module.def("make_market_order_reactor",
    [] (VirtualOrderExecutionClient& client,
        Aspen::SharedBox<Security> security, Aspen::SharedBox<Side> side,
        Aspen::SharedBox<Quantity> quantity) {
      return Aspen::to_object(MakeMarketOrderReactor(Ref(client),
        std::move(security), std::move(side), std::move(quantity)));
    });
}

void Nexus::Python::ExportOrderRecord(pybind11::module& module) {
  class_<OrderRecord>(module, "OrderRecord")
    .def(init())
    .def(init<OrderInfo, std::vector<ExecutionReport>>())
    .def(init<const OrderRecord&>())
    .def_readwrite("info", &OrderRecord::m_info)
    .def_readwrite("execution_reports", &OrderRecord::m_executionReports)
    .def(self == self)
    .def(self != self)
    .def("__str__", lexical_cast<std::string, OrderRecord>);
  ExportQueueSuite<OrderRecord>(module, "OrderRecord");
  ExportQueueSuite<SequencedOrderRecord>(module, "SequencedOrderRecord");
}

void Nexus::Python::ExportOrderWrapperReactor(pybind11::module& module) {
  Aspen::export_reactor<OrderWrapperReactor>(module, "OrderWrapperReactor")
    .def(init<Ref<const Order>>());
}

void Nexus::Python::ExportPrimitiveOrder(pybind11::module& module) {
  class_<PrimitiveOrder, Order>(module, "PrimitiveOrder")
    .def(init<OrderInfo>())
    .def(init<OrderRecord>())
    .def("update", &PrimitiveOrder::Update, call_guard<GilRelease>());
  module.def("build_rejected_order", &BuildRejectedOrder);
  module.def("reject_cancel_request", &RejectCancelRequest);
  ExportPublisher<const PrimitiveOrder*>(module,
    "ConstPrimitiveOrderPublisher");
  ExportPublisher<PrimitiveOrder*>(module, "PrimitiveOrderPublisher");
}

void Nexus::Python::ExportStandardQueries(pybind11::module& module) {
  module.def("build_daily_order_submission_query",
    &BuildDailyOrderSubmissionQuery);
  module.def("query_daily_order_submissions",
    &QueryDailyOrderSubmissions<VirtualOrderExecutionClient>);
}
