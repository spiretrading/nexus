#include "Nexus/Python/OrderExecutionService.hpp"
#include <Aspen/Python/Box.hpp>
#include <Beam/Python/Beam.hpp>
#include <Beam/Sql/SqlConnection.hpp>
#include <pybind11/operators.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/OrderExecutionService/ApplicationDefinitions.hpp"
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
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Python;
using namespace Nexus::Tests;
using namespace pybind11;

namespace {
  auto order_execution_client = std::unique_ptr<class_<OrderExecutionClient>>();
  auto order_execution_data_store =
    std::unique_ptr<class_<OrderExecutionDataStore>>();
}

class_<OrderExecutionClient>&
    Nexus::Python::get_exported_order_execution_client() {
  return *order_execution_client;
}

class_<OrderExecutionDataStore>&
    Nexus::Python::get_exported_order_execution_data_store() {
  return *order_execution_data_store;
}


void Nexus::Python::export_execution_report(module& module) {
  export_default_methods(class_<ExecutionReport>(module, "ExecutionReport")).
    def(init<OrderId, ptime>()).
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
    def_readwrite("additional_tags", &ExecutionReport::m_additional_tags);
  module.def("make_update", &make_update);
  module.def("fee_total", &get_fee_total);
  export_queue_suite<ExecutionReport>(module, "ExecutionReport");
  export_queue_suite<SequencedExecutionReport>(
    module, "SequencedExecutionReport");
  export_snapshot_publisher<ExecutionReport, std::vector<ExecutionReport>>(
    module, "ExecutionReport");
}

void Nexus::Python::export_execution_report_publisher(module& module) {
  export_default_methods(
      class_<ExecutionReportEntry>(module, "OrderExecutionReport")).
    def(init<std::shared_ptr<Order>, ExecutionReport>()).
    def_readwrite("order", &ExecutionReportEntry::m_order).
    def_readwrite("report", &ExecutionReportEntry::m_report);
  export_snapshot_publisher<
    ExecutionReportEntry, std::vector<ExecutionReportEntry>>(
      module, "BaseOrderExecutionReport");
  class_<ExecutionReportPublisher,
    SnapshotPublisher<ExecutionReportEntry, std::vector<ExecutionReportEntry>>,
    std::shared_ptr<ExecutionReportPublisher>>(
      module, "OrderExecutionReportPublisher").
        def(init<ScopedQueueReader<std::shared_ptr<Order>>>());
}

void Nexus::Python::export_local_order_execution_data_store(module& module) {
  export_order_execution_data_store<
      LocalOrderExecutionDataStore>(module, "LocalOrderExecutionDataStore").
    def(init()).
    def("load_order_submissions",
      &LocalOrderExecutionDataStore::load_order_submissions).
    def("load_execution_reports", overload_cast<>(
      &LocalOrderExecutionDataStore::load_execution_reports, const_));
}

void Nexus::Python::export_mock_order_execution_driver(module& module) {
  class_<MockOrderExecutionDriver, std::shared_ptr<MockOrderExecutionDriver>>(
      module, "MockOrderExecutionDriver").
    def(init(&make_python_shared<MockOrderExecutionDriver>)).
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
      return std::make_unique<ToPythonOrderExecutionDataStore<DataStore>>([=] {
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
  export_queue_suite<std::shared_ptr<Order>>(module, "Order");
  export_queue_suite<SequencedOrder>(module, "SequencedOrder");
  export_snapshot_publisher<std::shared_ptr<Order>,
    std::vector<std::shared_ptr<Order>>>(module, "Order");
}

void Nexus::Python::export_order_cancellation_reactor(module& module) {
  auto aspen_module = pybind11::module::import("aspen");
  export_box<std::shared_ptr<Order>>(aspen_module, "Order");
  export_reactor<OrderCancellationReactor<OrderExecutionClient*,
    SharedBox<std::shared_ptr<Order>>>>(module, "OrderCancellationReactor").def(
      init<OrderExecutionClient*, SharedBox<std::shared_ptr<Order>>>(),
    keep_alive<1, 2>());
}

void Nexus::Python::export_order_execution_data_store_exception(
    module& module) {
  register_exception<OrderExecutionDataStoreException>(module,
    "OrderExecutionDataStoreException", get_io_exception());
}

void Nexus::Python::export_order_execution_service(module& module) {
  order_execution_client = std::make_unique<class_<OrderExecutionClient>>(
    export_order_execution_client<OrderExecutionClient>(
      module, "OrderExecutionClient"));
  order_execution_data_store =
    std::make_unique<class_<OrderExecutionDataStore>>(
      export_order_execution_data_store<OrderExecutionDataStore>(
        module, "OrderExecutionDataStore"));
  export_execution_report(module);
  export_execution_report_publisher(module);
  export_local_order_execution_data_store(module);
  export_my_sql_order_execution_data_store(module);
  export_order(module);
  export_order_cancellation_reactor(module);
  export_order_execution_data_store_exception(module);
  export_order_execution_service_application_definitions(module);
  export_order_fields(module);
  export_order_info(module);
  export_order_reactor(module);
  export_order_record(module);
  export_order_wrapper_reactor(module);
  export_primitive_order(module);
  export_replicated_order_execution_data_store(module);
  export_standard_queries(module);
  export_sqlite_order_execution_data_store(module);
  auto test_module = module.def_submodule("tests");
  export_mock_order_execution_driver(test_module);
  export_order_execution_service_test_environment(test_module);
  test_module.def("cancel", overload_cast<PrimitiveOrder&, ptime>(&cancel),
    call_guard<GilRelease>());
  test_module.def("cancel", overload_cast<PrimitiveOrder&>(&cancel),
    call_guard<GilRelease>());
  test_module.def(
    "set_order_status", overload_cast<PrimitiveOrder&, OrderStatus, ptime>(
      &set_order_status), call_guard<GilRelease>());
  test_module.def("set_order_status",
    overload_cast<PrimitiveOrder&, OrderStatus>(&set_order_status),
    call_guard<GilRelease>());
  test_module.def("accept", overload_cast<PrimitiveOrder&, ptime>(&accept),
    call_guard<GilRelease>());
  test_module.def("accept", overload_cast<PrimitiveOrder&>(&accept),
    call_guard<GilRelease>());
  test_module.def("reject", overload_cast<PrimitiveOrder&, ptime>(&reject),
    call_guard<GilRelease>());
  test_module.def("reject", overload_cast<PrimitiveOrder&>(&reject),
    call_guard<GilRelease>());
  test_module.def(
    "fill", overload_cast<PrimitiveOrder&, Money, Quantity, ptime>(&fill),
    call_guard<GilRelease>());
  test_module.def(
    "fill", overload_cast<PrimitiveOrder&, Money, Quantity>(&fill),
    call_guard<GilRelease>());
  test_module.def(
    "fill", overload_cast<PrimitiveOrder&, Quantity, ptime>(&fill),
    call_guard<GilRelease>());
  test_module.def("fill",
    overload_cast<PrimitiveOrder&, Quantity>(&fill), call_guard<GilRelease>());
  test_module.def(
    "is_pending_cancel", &is_pending_cancel, call_guard<GilRelease>());
}

void Nexus::Python::export_order_execution_service_application_definitions(
    module& module) {
  export_order_execution_client<
    ToPythonOrderExecutionClient<ApplicationOrderExecutionClient>>(
      module, "ApplicationOrderExecutionClient").
    def(pybind11::init(
      [] (ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>&
          client) {
        return std::make_unique<ToPythonOrderExecutionClient<
          ApplicationOrderExecutionClient>>(Ref(client.get()));
      }), keep_alive<1, 2>());
}

void Nexus::Python::export_order_execution_service_test_environment(
    module& module) {
  class_<OrderExecutionServiceTestEnvironment,
      std::shared_ptr<OrderExecutionServiceTestEnvironment>>(
        module, "OrderExecutionServiceTestEnvironment").
    def(init(&make_python_shared<OrderExecutionServiceTestEnvironment,
      ServiceLocatorClient&, UidClient&, AdministrationClient&>),
      keep_alive<1, 2>(), keep_alive<1, 3>(), keep_alive<1, 4>()).
    def(init(&make_python_shared<OrderExecutionServiceTestEnvironment,
      const VenueDatabase&, const DestinationDatabase&, ServiceLocatorClient&,
      UidClient&, AdministrationClient&>), keep_alive<1, 4>(), keep_alive<1, 5>(),
      keep_alive<1, 6>()).
    def_property_readonly("data_store",
      overload_cast<>(&OrderExecutionServiceTestEnvironment::get_data_store),
      return_value_policy::reference_internal).
    def_property_readonly("driver",
      [] (OrderExecutionServiceTestEnvironment& self) -> auto& {
        return self.get_driver().as<MockOrderExecutionDriver>();
      }, return_value_policy::reference_internal).
    def("make_client", [] (OrderExecutionServiceTestEnvironment& self,
        ServiceLocatorClient& client) {
      return ToPythonOrderExecutionClient(self.make_client(Ref(client)));
    }, call_guard<GilRelease>(), keep_alive<0, 2>()).
    def("close", &OrderExecutionServiceTestEnvironment::close,
      call_guard<GilRelease>());
  module.def("make_order_execution_service_test_environment",
    &make_order_execution_service_test_environment, call_guard<GilRelease>());
}

void Nexus::Python::export_order_fields(module& module) {
  export_default_methods(class_<OrderFields>(module, "OrderFields")).
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
    def_readwrite("additional_fields", &OrderFields::m_additional_fields);
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, CurrencyId, Side, Destination,
      Quantity, Money>(&make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<Security, CurrencyId, Side, Destination, Quantity, Money>(
      &make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Destination, Quantity, Money>(
      &make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<Security, Side, Destination, Quantity, Money>(
      &make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<Security, CurrencyId, Side, Quantity, Money>(
      &make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Quantity, Money>(
      &make_limit_order_fields));
  module.def("make_limit_order_fields",
    overload_cast<Security, Side, Quantity, Money>(&make_limit_order_fields));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, CurrencyId, Side, Destination,
      Quantity>(&make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<Security, CurrencyId, Side, Destination, Quantity>(
      &make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Destination, Quantity>(
      &make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<Security, Side, Destination, Quantity>(
      &make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<Security, CurrencyId, Side, Quantity>(
      &make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<DirectoryEntry, Security, Side, Quantity>(
      &make_market_order_fields));
  module.def("make_market_order_fields",
    overload_cast<Security, Side, Quantity>(&make_market_order_fields));
  module.def("find_field", &find_field);
  module.def("has_field", &has_field);
}

void Nexus::Python::export_order_info(module& module) {
  export_default_methods(class_<OrderInfo>(module, "OrderInfo")).
    def(init<OrderFields, DirectoryEntry, OrderId, bool, ptime>()).
    def(init<OrderFields, OrderId, bool, ptime>()).
    def(init<OrderFields, OrderId, ptime>()).
    def_readwrite("fields", &OrderInfo::m_fields).
    def_readwrite("submission_account", &OrderInfo::m_submission_account).
    def_readwrite("id", &OrderInfo::m_id).
    def_readwrite("shorting_flag", &OrderInfo::m_shorting_flag).
    def_readwrite("timestamp", &OrderInfo::m_timestamp);
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
  export_reactor<OrderReactor<OrderExecutionClient*, SharedBox<DirectoryEntry>,
    SharedBox<Security>, SharedBox<CurrencyId>, SharedBox<OrderType>,
    SharedBox<Side>, SharedBox<std::string>, SharedBox<Quantity>,
    SharedBox<Money>, SharedBox<TimeInForce>, SharedBox<Tag>>>(
      module, "OrderReactor").
    def(init<OrderExecutionClient*, SharedBox<DirectoryEntry>,
      SharedBox<Security>, SharedBox<CurrencyId>, SharedBox<OrderType>,
      SharedBox<Side>, SharedBox<std::string>, SharedBox<Quantity>,
      SharedBox<Money>, SharedBox<TimeInForce>, std::vector<SharedBox<Tag>>>(),
      keep_alive<1, 2>());
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient& client, SharedBox<DirectoryEntry> account,
        SharedBox<Security> security, SharedBox<CurrencyId> currency,
        SharedBox<Side> side, SharedBox<std::string> destination,
        SharedBox<Quantity> quantity, SharedBox<Money> price,
        SharedBox<TimeInForce> time_in_force) {
      return to_object(make_limit_order_reactor(client,
        std::move(account), std::move(security), std::move(currency),
        std::move(side), std::move(destination), std::move(quantity),
        std::move(price), std::move(time_in_force)));
    }, keep_alive<0, 1>());
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient& client, SharedBox<DirectoryEntry> account,
        SharedBox<Security> security, SharedBox<CurrencyId> currency,
        SharedBox<Side> side, SharedBox<std::string> destination,
        SharedBox<Quantity> quantity, SharedBox<Money> price) {
      return to_object(make_limit_order_reactor(client, std::move(account),
        std::move(security), std::move(currency), std::move(side),
        std::move(destination), std::move(quantity), std::move(price)));
    }, keep_alive<0, 1>());
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient& client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity,
        SharedBox<Money> price) {
      return to_object(make_limit_order_reactor(client, std::move(security),
        std::move(side), std::move(quantity), std::move(price)));
    }, keep_alive<0, 1>());
  module.def("make_limit_order_reactor",
    [] (OrderExecutionClient& client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity,
        SharedBox<Money> price, SharedBox<TimeInForce> time_in_force) {
      return to_object(make_limit_order_reactor(client, std::move(security),
        std::move(side), std::move(quantity), std::move(price),
        std::move(time_in_force)));
    }, keep_alive<0, 1>());
  module.def("make_market_order_reactor",
    [] (OrderExecutionClient& client, SharedBox<Security> security,
        SharedBox<Side> side, SharedBox<Quantity> quantity) {
      return to_object(make_market_order_reactor(client, std::move(security),
        std::move(side), std::move(quantity)));
    }, keep_alive<0, 1>());
}

void Nexus::Python::export_order_record(module& module) {
  export_default_methods(class_<OrderRecord>(module, "OrderRecord")).
    def_readwrite("info", &OrderRecord::m_info).
    def_readwrite("execution_reports", &OrderRecord::m_execution_reports);
  export_queue_suite<OrderRecord>(module, "OrderRecord");
  export_queue_suite<SequencedOrderRecord>(module, "SequencedOrderRecord");
}

void Nexus::Python::export_order_wrapper_reactor(module& module) {
  export_reactor<OrderWrapperReactor>(module, "OrderWrapperReactor").
    def(init<std::shared_ptr<Order>>());
}

void Nexus::Python::export_primitive_order(module& module) {
  class_<PrimitiveOrder, Order, std::shared_ptr<PrimitiveOrder>>(
      module, "PrimitiveOrder").
    def(init<OrderInfo>()).
    def(init<OrderRecord>()).
    def("update", &PrimitiveOrder::update).
    def("with", [] (PrimitiveOrder& self, PythonFunction<
        object (OrderStatus, const std::vector<ExecutionReport>&)> f) {
      return self.with([&] (auto status, const auto& reports) {
        return f(status, reports);
      });
    });
  module.def("make_rejected_order", &make_rejected_order);
  module.def("reject_cancel_request", &reject_cancel_request);
  export_publisher<std::shared_ptr<const PrimitiveOrder>>(
    module, "ConstPrimitiveOrderPublisher");
  export_publisher<std::shared_ptr<PrimitiveOrder>>(
    module, "PrimitiveOrderPublisher");
}

void Nexus::Python::export_replicated_order_execution_data_store(
    module& module) {
  export_order_execution_data_store<ReplicatedOrderExecutionDataStore>(
    module, "ReplicatedOrderExecutionDataStore").def(
      init<OrderExecutionDataStore, std::vector<OrderExecutionDataStore>>());
}

void Nexus::Python::export_standard_queries(module& module) {
  module.def("make_venue_filter", &make_venue_filter);
  module.def(
    "make_daily_order_submission_query", &make_daily_order_submission_query);
  module.def("query_daily_order_submissions",
    [] (const DirectoryEntry& account, ptime start, ptime end,
        const VenueDatabase& venues, const local_time::tz_database& time_zones,
        SharedObject shared_client,
        ScopedQueueWriter<std::shared_ptr<Order>> queue) {
      return spawn([=, queue = std::move(queue)] mutable {
        auto& client = [&] () -> auto& {
          auto lock = GilLock();
          return shared_client->cast<OrderExecutionClient&>();
        }();
        auto query = RoutineHandler(query_daily_order_submissions(
          account, start, end, venues, time_zones, client, std::move(queue)));
        query.wait();
      });
    });
  module.def("make_live_orders_filter", &make_live_orders_filter);
  module.def("make_live_orders_query", &make_live_orders_query);
  module.def("query_live_orders",
    [] (const DirectoryEntry& account, OrderExecutionClient& client,
        ScopedQueueWriter<std::shared_ptr<Order>> queue) {
      return query_live_orders(account, client, std::move(queue));
    });
  module.def("load_live_orders",
    [] (const DirectoryEntry& account, OrderExecutionClient& client) {
      return load_live_orders(account, client);
    }, call_guard<GilRelease>());
  module.def("make_order_id_filter", &make_order_id_filter);
  module.def("make_order_id_query", &make_order_id_query);
  module.def("query_order_ids",
    [] (const DirectoryEntry& account, const std::vector<OrderId>& ids,
        OrderExecutionClient& client,
        ScopedQueueWriter<std::shared_ptr<Order>> queue) {
      return query_order_ids(account, ids, client, std::move(queue));
    });
  module.def("load_orders",
    [] (const DirectoryEntry& account, const std::vector<OrderId>& ids,
        OrderExecutionClient& client) {
      return load_orders(account, ids, client);
    }, call_guard<GilRelease>());
}

void Nexus::Python::export_sqlite_order_execution_data_store(module& module) {
  using DataStore =
    SqlOrderExecutionDataStore<SqlConnection<Viper::Sqlite3::Connection>>;
  export_order_execution_data_store<ToPythonOrderExecutionDataStore<DataStore>>(
    module, "SqliteOrderExecutionDataStore").
      def(init([] (std::string path) {
        return std::make_unique<ToPythonOrderExecutionDataStore<DataStore>>(
          [=] {
            auto release = GilRelease();
            return SqlConnection(Viper::Sqlite3::Connection(path));
          });
      }));
}
