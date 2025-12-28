#ifndef NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported OrderExecutionClient. */
  NEXUS_EXPORT_DLL pybind11::class_<OrderExecutionClient>&
    get_exported_order_execution_client();

  /** Returns the exported OrderExecutionDataStore. */
  NEXUS_EXPORT_DLL pybind11::class_<OrderExecutionDataStore>&
    get_exported_order_execution_data_store();

  /**
   * Exports the ExecutionReport struct.
   * @param module The module to export to.
   */
  void export_execution_report(pybind11::module& module);

  /**
   * Exports the ExecutionReportPublisher class.
   * @param module The module to export to.
   */
  void export_execution_report_publisher(pybind11::module& module);

  /**
   * Exports the LocalOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void export_local_order_execution_data_store(pybind11::module& module);

  /**
   * Exports the MockOrderExecutionDriver class.
   * @param module The module to export to.
   */
  void export_mock_order_execution_driver(pybind11::module& module);

  /**
   * Exports the MySqlOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void export_my_sql_order_execution_data_store(pybind11::module& module);

  /**
   * Exports the Order class.
   * @param module The module to export to.
   */
  void export_order(pybind11::module& module);

  /**
   * Exports the OrderCancellationReactor class.
   * @param module The module to export to.
   */
  void export_order_cancellation_reactor(pybind11::module& module);

  /**
   * Exports an OrderExecutionClient class.
   * @param <C> The type of OrderExecutionClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported OrderExecutionClient.
   */
  template<IsOrderExecutionClient C>
  auto export_order_execution_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C, std::shared_ptr<C>>(module, name.data()).
      def("load_order", &C::load_order).
      def("query_sequenced_order_records", pybind11::overload_cast<
        const AccountQuery&, Beam::ScopedQueueWriter<SequencedOrderRecord>>(
          &C::query)).
      def("query_order_records", pybind11::overload_cast<
        const AccountQuery&, Beam::ScopedQueueWriter<OrderRecord>>(&C::query)).
      def("query_sequenced_orders", pybind11::overload_cast<
        const AccountQuery&, Beam::ScopedQueueWriter<SequencedOrder>>(
          &C::query)).
      def("query_orders", pybind11::overload_cast<const AccountQuery&,
        Beam::ScopedQueueWriter<std::shared_ptr<Order>>>(&C::query)).
      def("query_sequenced_execution_reports", pybind11::overload_cast<
        const AccountQuery&, Beam::ScopedQueueWriter<SequencedExecutionReport>>(
          &C::query)).
      def("query_execution_reports", pybind11::overload_cast<
        const AccountQuery&, Beam::ScopedQueueWriter<ExecutionReport>>(
          &C::query)).
      def("submit", &C::submit).
      def("cancel", pybind11::overload_cast<const Order&>(&C::cancel)).
      def("update", &C::update).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, OrderExecutionClient>) {
      pybind11::implicitly_convertible<C, OrderExecutionClient>();
      get_exported_order_execution_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /**
   * Exports an OrderExecutionDataStore class.
   * @param <D> The type of OrderExecutionDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported OrderExecutionDataStore.
   */
  template<IsOrderExecutionDataStore D>
  auto export_order_execution_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<D>(module, name.data()).
      def("load_order_record", &D::load_order_record).
      def("load_order_records", pybind11::overload_cast<const AccountQuery&>(
        &D::load_order_records)).
      def("store", pybind11::overload_cast<const SequencedAccountOrderInfo&>(
        &D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedAccountOrderInfo>&>(&D::store)).
      def("load_execution_reports",
        pybind11::overload_cast<const AccountQuery&>(
          &D::load_execution_reports)).
      def("store",
        pybind11::overload_cast<const SequencedAccountExecutionReport&>(
          &D::store)).
      def("store", pybind11::overload_cast<
        const std::vector<SequencedAccountExecutionReport>&>(&D::store)).
      def("close", &D::close);
    if constexpr(!std::is_same_v<D, OrderExecutionDataStore>) {
      pybind11::implicitly_convertible<D, OrderExecutionDataStore>();
      get_exported_order_execution_data_store().
        def(pybind11::init<D*>(), pybind11::keep_alive<1, 2>());
    }
    return data_store;
  }

  /**
   * Exports the OrderExecutionDataStoreException class.
   * @param module The module to export to.
   */
  void export_order_execution_data_store_exception(pybind11::module& module);

  /**
   * Exports the OrderExecutionService namespace.
   * @param module The module to export to.
   */
  void export_order_execution_service(pybind11::module& module);

  /**
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_order_execution_service_application_definitions(
    pybind11::module& module);

  /**
   * Exports the OrderExecutionServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_order_execution_service_test_environment(
    pybind11::module& module);

  /**
   * Exports the OrderFields struct.
   * @param module The module to export to.
   */
  void export_order_fields(pybind11::module& module);

  /**
   * Exports the OrderInfo struct.
   * @param module The module to export to.
   */
  void export_order_info(pybind11::module& module);

  /**
   * Exports the OrderReactor class.
   * @param module The module to export to.
   */
  void export_order_reactor(pybind11::module& module);

  /**
   * Exports the OrderRecord struct.
   * @param module The module to export to.
   */
  void export_order_record(pybind11::module& module);

  /**
   * Exports the OrderWrapperReactor class.
   * @param module The module to export to.
   */
  void export_order_wrapper_reactor(pybind11::module& module);

  /**
   * Exports the PrimitiveOrder class.
   * @param module The module to export to.
   */
  void export_primitive_order(pybind11::module& module);

  /**
   * Exports the ReplicatedOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void export_replicated_order_execution_data_store(
    pybind11::module& module);

  /**
   * Exports the standard queries.
   * @param module The module to export to.
   */
  void export_standard_queries(pybind11::module& module);

  /**
   * Exports the SqliteOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void export_sqlite_order_execution_data_store(pybind11::module& module);
}

#endif
