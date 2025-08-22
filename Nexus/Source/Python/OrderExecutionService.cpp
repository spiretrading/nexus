#include "Nexus/Python/OrderExecutionService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <pybind11/operators.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/OrderExecutionService/ExecutionReportPublisher.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreException.hpp"
#include "Nexus/OrderExecutionService/OrderReactor.hpp"
#include "Nexus/OrderExecutionService/OrderWrapperReactor.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionService/ReplicatedOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/SqlOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTestEnvironment.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"
#include "Nexus/Python/ToPythonOrderExecutionClient.hpp"
#include "Nexus/Python/ToPythonOrderExecutionDataStore.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace pybind11;

void Nexus::Python::export_execution_report(module& module) {
  class_<ExecutionReport>(module, "ExecutionReport").
    def(init()).
    def(init<const ExecutionReport&>()).
    def(init<OrderId, boost::posix_time::ptime>()).
    def_readwrite("id", &ExecutionReport::m_id).
    def_readwrite("timestamp", &ExecutionReport::m_timestamp).
    def_readwrite("sequence", &ExecutionReport::m_sequence).
    def_readwrite("status", &ExecutionReport::m_status).
    def_readwrite("last_quantity", &ExecutionReport::m_last_quantity).
    def_readwrite("last_price", &ExecutionReport::m_last_price).
    def_readwrite("liquidity_flag", &ExecutionReport::m_liquidity_flag).
    def_readwrite("last_market", &ExecutionReport::m_last_market).
    def_readwrite("execution_fee", &ExecutionReport::m_execution_fee).
    def_readwrite("processing_fee", &ExecutionReport::m_processing_fee).
    def_readwrite("commission", &ExecutionReport::m_commission).
    def_readwrite("text", &ExecutionReport::m_text).
    def_readwrite("additional_tags", &ExecutionReport::m_additional_tags).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, ExecutionReport>);
  module.def("make_update", &make_update, arg("report"), arg("status"),
    arg("timestamp"));
  module.def("fee_total", &get_fee_total, arg("report"));
  ExportQueueSuite<ExecutionReport>(module, "ExecutionReport");
  ExportQueueSuite<SequencedExecutionReport>(
    module, "SequencedExecutionReport");
  ExportSnapshotPublisher<ExecutionReport, std::vector<ExecutionReport>>(
    module, "ExecutionReport");
}

void Nexus::Python::export_execution_report_publisher(module& module) {
  class_<ExecutionReportEntry>(module, "OrderExecutionReport").
    def(init()).
    def(init<const ExecutionReportEntry&>()).
    def(init<std::shared_ptr<const Order>, ExecutionReport>()).
    def_readwrite("order", &ExecutionReportEntry::m_order).
    def_readwrite("report", &ExecutionReportEntry::m_report);
  ExportSnapshotPublisher<
    ExecutionReportEntry, std::vector<ExecutionReportEntry>>(
      module, "BaseOrderExecutionReport");
  class_<ExecutionReportPublisher,
    SnapshotPublisher<ExecutionReportEntry, std::vector<ExecutionReportEntry>>,
    std::shared_ptr<ExecutionReportPublisher>>(
      module, "OrderExecutionReportPublisher").
        def(init<ScopedQueueReader<std::shared_ptr<const Order>>>());
}

void Nexus::Python::export_local_order_execution_data_store(module& module) {
  auto data_store = export_order_execution_data_store<
    LocalOrderExecutionDataStore>(module, "LocalOrderExecutionDataStore");
  data_store.
    def(init()).
    def("load_order_submissions",
      &LocalOrderExecutionDataStore::load_order_submissions).
    def("load_execution_reports", static_cast<
      std::vector<SequencedAccountExecutionReport> (
        LocalOrderExecutionDataStore::*)() const>(
          &LocalOrderExecutionDataStore::load_execution_reports));
}

void Nexus::Python::export_mock_order_execution_driver(module& module) {
  class_<MockOrderExecutionDriver>(module, "MockOrderExecutionDriver").
    def(init()).
    def("__del__", [] (MockOrderExecutionDriver& self) {
      auto release = GilRelease();
      self.close();
    }).
    def("set_order_status_new_on_submission",
      &MockOrderExecutionDriver::set_order_status_new_on_submission).
    def("find", &MockOrderExecutionDriver::find,
      return_value_policy::reference_internal).
    def("add_recovery", &MockOrderExecutionDriver::add_recovery).
    def("get_publisher", &MockOrderExecutionDriver::get_publisher,
      return_value_policy::reference_internal).
    def("recover", &MockOrderExecutionDriver::recover).
    def("add", &MockOrderExecutionDriver::add).
    def("submit", &MockOrderExecutionDriver::submit, call_guard<GilRelease>()).
    def("cancel", &MockOrderExecutionDriver::cancel, call_guard<GilRelease>()).
    def("update", &MockOrderExecutionDriver::update, call_guard<GilRelease>()).
    def("close", &MockOrderExecutionDriver::close, call_guard<GilRelease>());
}

void Nexus::Python::export_my_sql_order_execution_data_store(module& module) {
  using DataStore =
    SqlOrderExecutionDataStore<SqlConnection<Viper::MySql::Connection>>;
  export_order_execution_data_store<ToPythonOrderExecutionDataStore<DataStore>>(
      module, "MySqlOrderExecutionDataStore").
    def(init([] (std::string host, unsigned int port, std::string username,
        std::string password, std::string database) {
      return std::make_shared<ToPythonOrderExecutionDataStore<DataStore>>([=] {
        auto release = GilRelease();
        return SqlConnection(Viper::MySql::Connection(host, port, username,
          password, database));
      });
    }));
}

void Nexus::Python::export_order(module& module) {
  class_<Order, std::shared_ptr<Order>>(module, "Order").
    def_property_readonly("info", &Order::get_info).
    def_property_readonly("publisher", &Order::get_publisher,
      return_value_policy::reference_internal);
  ExportQueueSuite<std::shared_ptr<const Order>>(module, "Order");
  ExportQueueSuite<SequencedOrder>(module, "SequencedOrder");
  ExportSnapshotPublisher<std::shared_ptr<const Order>,
    std::vector<std::shared_ptr<const Order>>>(module, "Order");
}

void Nexus::Python::export_order_cancellation_reactor(module& module) {
  auto aspen_module = pybind11::module::import("aspen");
  export_box<std::shared_ptr<const Order>>(aspen_module, "Order");
  export_reactor<OrderCancellationReactor<OrderExecutionClient,
    SharedBox<std::shared_ptr<const Order>>>>(
      module, "OrderCancellationReactor").def(
        init<OrderExecutionClient, SharedBox<std::shared_ptr<const Order>>>());
}

void Nexus::Python::export_order_execution_data_store_exception(
    module& module) {
  register_exception<OrderExecutionDataStoreException>(module,
    "OrderExecutionDataStoreException", GetIOException());
}

void Nexus::Python::export_order_execution_service(module& module) {
  auto submodule = module.def_submodule("order_execution_service");
  export_execution_report(submodule);
  export_execution_report_publisher(submodule);
  export_local_order_execution_data_store(submodule);
  export_my_sql_order_execution_data_store(submodule);
  export_order(submodule);
  export_order_cancellation_reactor(submodule);
  export_order_execution_client<ToPythonOrderExecutionClient<
    OrderExecutionClient>>(submodule, "OrderExecutionClient");
  export_order_execution_data_store<ToPythonOrderExecutionDataStore<
    OrderExecutionDataStore>>(submodule, "OrderExecutionDataStore");
  export_order_execution_data_store_exception(submodule);
  export_order_fields(submodule);
  export_order_info(submodule);
  export_order_reactor(submodule);
  export_order_record(submodule);
  export_order_wrapper_reactor(submodule);
  export_primitive_order(submodule);
  export_replicated_order_execution_data_store(submodule);
  export_standard_queries(submodule);
  export_sqlite_order_execution_data_store(submodule);
  auto test_module = submodule.def_submodule("tests");
  export_mock_order_execution_driver(test_module);
  export_order_execution_service_test_environment(test_module);
  test_module.def("cancel",
    overload_cast<PrimitiveOrder&, boost::posix_time::ptime>(&cancel),
    call_guard<GilRelease>(), arg("order"), arg("timestamp"));
  test_module.def(
    "cancel", overload_cast<PrimitiveOrder&>(&cancel), call_guard<GilRelease>(),
    arg("order"));
  test_module.def("set_order_status",
    overload_cast<PrimitiveOrder&, OrderStatus, boost::posix_time::ptime>(
      &set_order_status), call_guard<GilRelease>(), arg("order"),
    arg("new_status"), arg("timestamp"));
  test_module.def("set_order_status",
    overload_cast<PrimitiveOrder&, OrderStatus>(&set_order_status),
    call_guard<GilRelease>(), arg("order"), arg("new_status"));
  test_module.def("accept",
    overload_cast<PrimitiveOrder&, boost::posix_time::ptime>(&accept),
    call_guard<GilRelease>(), arg("order"), arg("timestamp"));
  test_module.def("accept",
    overload_cast<PrimitiveOrder&>(&accept), call_guard<GilRelease>(),
    arg("order"));
  test_module.def("reject",
    overload_cast<PrimitiveOrder&, boost::posix_time::ptime>(&reject),
    call_guard<GilRelease>(), arg("order"), arg("timestamp"));
  test_module.def("reject",
    overload_cast<PrimitiveOrder&>(&reject), call_guard<GilRelease>(),
    arg("order"));
  test_module.def("fill",
    overload_cast<PrimitiveOrder&, Money, Quantity,
      boost::posix_time::ptime>(&fill), call_guard<GilRelease>(),
    arg("order"), arg("price"), arg("quantity"), arg("timestamp"));
  test_module.def("fill",
    overload_cast<PrimitiveOrder&, Money, Quantity>(&fill),
    call_guard<GilRelease>(), arg("order"), arg("price"), arg("quantity"));
  test_module.def("fill",
    overload_cast<PrimitiveOrder&, Quantity, boost::posix_time::ptime>(&fill),
    call_guard<GilRelease>(), arg("order"), arg("quantity"), arg("timestamp"));
  test_module.def("fill",
    overload_cast<PrimitiveOrder&, Quantity>(&fill), call_guard<GilRelease>(),
    arg("order"), arg("quantity"));
  test_module.def("is_pending_cancel", &is_pending_cancel,
    call_guard<GilRelease>(), arg("order"));
}

void Nexus::Python::export_order_execution_service_test_environment(
    module& module) {
  class_<OrderExecutionServiceTestEnvironment>(
      module, "OrderExecutionServiceTestEnvironment").
    def(init<ServiceLocatorClientBox, UidClientBox, AdministrationClient>(),
      call_guard<GilRelease>()).
    def(init<const VenueDatabase&, const DestinationDatabase&,
      ServiceLocatorClientBox, UidClientBox, AdministrationClient>(),
      call_guard<GilRelease>()).
    def("__del__", [] (OrderExecutionServiceTestEnvironment& self) {
      self.close();
    }, call_guard<GilRelease>()).
    def_property_readonly("data_store",
      [] (OrderExecutionServiceTestEnvironment& self) {
        return ToPythonOrderExecutionDataStore(&self.get_data_store());
      }, keep_alive<0, 1>()).
    def_property_readonly("driver",
      [] (OrderExecutionServiceTestEnvironment& self) -> auto& {
        return self.get_driver().as<MockOrderExecutionDriver>();
      }, return_value_policy::reference_internal).
    def("make_client",
      [] (OrderExecutionServiceTestEnvironment& self,
          ServiceLocatorClientBox service_locator_client) {
        return ToPythonOrderExecutionClient(
          self.make_client(std::move(service_locator_client)));
      }, call_guard<GilRelease>()).
    def("close", &OrderExecutionServiceTestEnvironment::close,
      call_guard<GilRelease>());
  module.def("make_order_execution_service_test_environment",
    &make_order_execution_service_test_environment, call_guard<GilRelease>());
}

void Nexus::Python::export_order_fields(module& module) {
  class_<OrderFields>(module, "OrderFields").
    def(init()).
    def(init<const OrderFields&>()).
    def(init<DirectoryEntry, Security, CurrencyId, OrderType, Side, Destination,
      Quantity, Money, TimeInForce, std::vector<Tag>>()).
    def_readwrite("account", &OrderFields::m_account).
    def_readwrite("security", &OrderFields::m_security).
    def_readwrite("currency", &OrderFields::m_currency).
    def_readwrite("type", &OrderFields::m_type).
    def_readwrite("side", &OrderFields::m_side).
    def_readwrite("destination", &OrderFields::m_destination).
    def_readwrite("quantity", &OrderFields::m_quantity).
    def_readwrite("price", &OrderFields::m_price).
    def_readwrite("time_in_force", &OrderFields::m_time_in_force).
    def_readwrite("additional_fields", &OrderFields::m_additional_fields).
    def(self == self).
    def(self != self).
    def(self < self).
    def("__str__", &boost::lexical_cast<std::string, OrderFields>);
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, CurrencyId, Side, Destination,
      Quantity, Money>(&make_limit_order_fields), arg("account"),
    arg("security"), arg("currency"), arg("side"), arg("destination"),
    arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<Security, CurrencyId, Side, Destination, Quantity, Money>(
      &make_limit_order_fields), arg("security"), arg("currency"), arg("side"),
    arg("destination"), arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Destination, Quantity, Money>(
      &make_limit_order_fields), arg("account"), arg("security"), arg("side"),
    arg("destination"), arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<Security, Side, Destination, Quantity, Money>(
      &make_limit_order_fields), arg("security"), arg("side"),
    arg("destination"), arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<Security, CurrencyId, Side, Quantity, Money>(
      &make_limit_order_fields), arg("security"), arg("currency"), arg("side"),
    arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Quantity, Money>(
      &make_limit_order_fields), arg("account"), arg("security"), arg("side"),
    arg("quantity"), arg("price"));
  module.def("make_limit_order_fields",
    overload_cast<Security, Side, Quantity, Money>(&make_limit_order_fields),
    arg("security"), arg("side"), arg("quantity"), arg("price"));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, CurrencyId, Side, Destination,
      Quantity>(&make_market_order_fields), arg("account"), arg("security"),
    arg("currency"), arg("side"), arg("destination"), arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<Security, CurrencyId, Side, Destination, Quantity>(
      &make_market_order_fields), arg("security"), arg("currency"), arg("side"),
    arg("destination"), arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Destination, Quantity>(
      &make_market_order_fields), arg("account"), arg("security"), arg("side"),
    arg("destination"), arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<Security, Side, Destination, Quantity>(
      &make_market_order_fields), arg("security"), arg("side"),
    arg("destination"), arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<Security, CurrencyId, Side, Quantity>(
      &make_market_order_fields), arg("security"), arg("currency"), arg("side"),
    arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Quantity>(
      &make_market_order_fields), arg("account"), arg("security"), arg("side"),
    arg("quantity"));
  module.def("make_market_order_fields",
    overload_cast<Security, Side, Quantity>(&make_market_order_fields),
    arg("security"), arg("side"), arg("quantity"));
  module.def("find_field", &find_field, arg("fields"), arg("key"));
  module.def("has_field", &has_field, arg("fields"), arg("tag"));
}

void Nexus::Python::export_order_info(module& module) {
  class_<OrderInfo>(module, "OrderInfo").
    def(init()).
    def(init<const OrderInfo&>()).
    def(init<OrderFields, DirectoryEntry, OrderId, bool,
      boost::posix_time::ptime>()).
    def(init<OrderFields, OrderId, bool, boost::posix_time::ptime>()).
    def(init<OrderFields, OrderId, boost::posix_time::ptime>()).
    def_readwrite("fields", &OrderInfo::m_fields).
    def_readwrite("submission_account", &OrderInfo::m_submission_account).
    def_readwrite("id", &OrderInfo::m_id).
    def_readwrite("shorting_flag", &OrderInfo::m_shorting_flag).
    def_readwrite("timestamp", &OrderInfo::m_timestamp).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, OrderInfo>);
}

void Nexus::Python::export_order_reactor(module& module) {
  auto aspen_module = pybind11::module::import("aspen");
  export_box<CurrencyId>(aspen_module, "CurrencyId");
  export_box<OrderType>(aspen_module, "OrderType");
  export_box<Side>(aspen_module, "Side");
  export_box<Quantity>(aspen_module, "Quantity");
  export_box<Money>(aspen_module, "Money");
  export_box<TimeInForce>(aspen_module, "TimeInForce");
  export_box<Tag>(aspen_module, "Tag");
  export_reactor<OrderReactor<OrderExecutionClient, SharedBox<DirectoryEntry>,
    SharedBox<Security>, SharedBox<CurrencyId>, SharedBox<OrderType>,
    SharedBox<Side>, SharedBox<std::string>, SharedBox<Quantity>,
    SharedBox<Money>, SharedBox<TimeInForce>, SharedBox<Tag>>>(
      module, "OrderReactor").
    def(init<OrderExecutionClient, SharedBox<DirectoryEntry>,
      SharedBox<Security>, SharedBox<CurrencyId>, SharedBox<OrderType>,
      SharedBox<Side>, SharedBox<std::string>, SharedBox<Quantity>,
      SharedBox<Money>, SharedBox<TimeInForce>, std::vector<SharedBox<Tag>>>());
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient client, SharedBox<DirectoryEntry> account,
        SharedBox<Security> security, SharedBox<CurrencyId> currency,
        SharedBox<Side> side, SharedBox<std::string> destination,
        SharedBox<Quantity> quantity, SharedBox<Money> price,
        SharedBox<TimeInForce> time_in_force) {
      return to_object(make_limit_order_reactor(std::move(client),
        std::move(account), std::move(security), std::move(currency),
        std::move(side), std::move(destination), std::move(quantity),
        std::move(price), std::move(time_in_force)));
    });
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient client, SharedBox<DirectoryEntry> account,
        SharedBox<Security> security, SharedBox<CurrencyId> currency,
        SharedBox<Side> side, SharedBox<std::string> destination,
        SharedBox<Quantity> quantity, SharedBox<Money> price) {
      return to_object(make_limit_order_reactor(std::move(client),
        std::move(account), std::move(security), std::move(currency),
        std::move(side), std::move(destination), std::move(quantity),
        std::move(price)));
    });
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity,
        SharedBox<Money> price) {
      return to_object(make_limit_order_reactor(std::move(client),
        std::move(security), std::move(side), std::move(quantity),
        std::move(price)));
    });
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity,
        SharedBox<Money> price, SharedBox<TimeInForce> time_in_force) {
      return to_object(make_limit_order_reactor(std::move(client),
        std::move(security), std::move(side), std::move(quantity),
        std::move(price), std::move(time_in_force)));
    });
  module.def("make_market_order_reactor",
    [] (OrderExecutionClient client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity) {
      return to_object(make_market_order_reactor(std::move(client),
        std::move(security), std::move(side), std::move(quantity)));
    });
}

void Nexus::Python::export_order_record(module& module) {
  class_<OrderRecord>(module, "OrderRecord").
    def(init()).
    def(init<const OrderRecord&>()).
    def_readwrite("info", &OrderRecord::m_info).
    def_readwrite("execution_reports", &OrderRecord::m_execution_reports).
    def(self == self).
    def(self != self).
    def("__str__", &boost::lexical_cast<std::string, OrderRecord>);
  ExportQueueSuite<OrderRecord>(module, "OrderRecord");
  ExportQueueSuite<SequencedOrderRecord>(module, "SequencedOrderRecord");
}

void Nexus::Python::export_order_wrapper_reactor(module& module) {
  export_reactor<OrderWrapperReactor>(module, "OrderWrapperReactor").
    def(init<std::shared_ptr<const Order>>());
}

void Nexus::Python::export_primitive_order(module& module) {
  class_<PrimitiveOrder, Order, std::shared_ptr<PrimitiveOrder>>(
      module, "PrimitiveOrder").
    def(init<OrderInfo>()).
    def(init<OrderRecord>()).
    def("update", &PrimitiveOrder::update).
    def("with", [] (PrimitiveOrder& self, function f) {
      return self.with([&] (auto status, const auto& reports) {
        return f(status, reports);
      });
    });
  module.def("make_rejected_order", &make_rejected_order, arg("info"),
    arg("reason"));
  module.def("reject_cancel_request", &reject_cancel_request, arg("order"),
    arg("timestamp"), arg("reason"));
  ExportPublisher<std::shared_ptr<const PrimitiveOrder>>(
    module, "ConstPrimitiveOrderPublisher");
  ExportPublisher<std::shared_ptr<PrimitiveOrder>>(
    module, "PrimitiveOrderPublisher");
}

void Nexus::Python::export_replicated_order_execution_data_store(
    module& module) {
  export_order_execution_data_store<ReplicatedOrderExecutionDataStore>(
    module, "ReplicatedOrderExecutionDataStore").def(
      init<OrderExecutionDataStore, std::vector<OrderExecutionDataStore>>());
}

void Nexus::Python::export_standard_queries(module& module) {
  module.def("make_venue_filter", &make_venue_filter, arg("venue"));
  module.def("make_daily_order_submission_query",
    &make_daily_order_submission_query, arg("venue"), arg("account"),
    arg("start"), arg("end"), arg("venues"), arg("time_zones"));
  module.def("query_daily_order_submissions",
    [] (const DirectoryEntry& account, boost::posix_time::ptime start,
        boost::posix_time::ptime end, const VenueDatabase& venues,
        const boost::local_time::tz_database& time_zones,
        OrderExecutionClient client,
        ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
      return query_daily_order_submissions(account, start, end, venues,
        time_zones, std::move(client), std::move(queue));
    }, arg("account"), arg("start"), arg("end"), arg("venues"),
    arg("time_zones"), arg("client"), arg("queue"));
  module.def("make_live_orders_filter", &make_live_orders_filter);
  module.def(
    "make_live_orders_query", &make_live_orders_query, arg("account"));
  module.def("query_live_orders",
    [] (const DirectoryEntry& account, OrderExecutionClient client,
        ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
      return query_live_orders(account, client, std::move(queue));
    }, arg("account"), arg("client"), arg("queue"));
  module.def("load_live_orders",
    [] (const DirectoryEntry& account, OrderExecutionClient client) {
      return load_live_orders(account, client);
    }, call_guard<GilRelease>(), arg("account"), arg("client"));
  module.def("make_order_id_filter", &make_order_id_filter, arg("ids"));
  module.def(
    "make_order_id_query", &make_order_id_query, arg("account"), arg("ids"));
  module.def("query_order_ids",
    [] (const DirectoryEntry& account, const std::vector<OrderId>& ids,
        OrderExecutionClient client,
        ScopedQueueWriter<std::shared_ptr<const Order>> queue) {
      return query_order_ids(account, ids, client, std::move(queue));
    }, arg("account"), arg("ids"), arg("client"), arg("queue"));
  module.def("load_orders",
    [] (const DirectoryEntry& account, const std::vector<OrderId>& ids,
        OrderExecutionClient client) {
      return load_orders(account, ids, client);
    }, call_guard<GilRelease>(), arg("account"), arg("ids"), arg("client"));
}

void Nexus::Python::export_sqlite_order_execution_data_store(module& module) {
  using DataStore =
    SqlOrderExecutionDataStore<SqlConnection<Viper::Sqlite3::Connection>>;
  export_order_execution_data_store<ToPythonOrderExecutionDataStore<DataStore>>(
    module, "SqliteOrderExecutionDataStore").
      def(init([] (std::string path) {
        return std::make_shared<ToPythonOrderExecutionDataStore<DataStore>>(
          [=] {
            auto release = GilRelease();
            return SqlConnection(Viper::Sqlite3::Connection(path));
          });
      }));
}
