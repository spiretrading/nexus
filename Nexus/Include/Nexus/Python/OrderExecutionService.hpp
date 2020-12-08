#ifndef NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#define NEXUS_PYTHON_ORDER_EXECUTION_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/OrderExecutionService/OrderExecutionClientBox.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported OrderExecutionClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<
    OrderExecutionService::OrderExecutionClientBox>&
      GetExportedOrderExecutionClientBox();

  /** Returns the exported OrderExecutionDataStoreBox. */
  NEXUS_EXPORT_DLL pybind11::class_<
    OrderExecutionService::OrderExecutionDataStoreBox>&
      GetExportedOrderExecutionDataStoreBox();

  /**
   * Exports the ApplicationOrderExecutionClient class.
   * @param module The module to export to.
   */
  void ExportApplicationOrderExecutionClient(pybind11::module& module);

  /**
   * Exports the ExecutionReport struct.
   * @param module The module to export to.
   */
  void ExportExecutionReport(pybind11::module& module);

  /**
   * Exports the LocalOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void ExportLocalOrderExecutionDataStore(pybind11::module& module);

  /**
   * Exports the MockOrderExecutionDriver class.
   * @param module The module to export to.
   */
  void ExportMockOrderExecutionDriver(pybind11::module& module);

  /**
   * Exports the MySqlOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void ExportMySqlOrderExecutionDataStore(pybind11::module& module);

  /**
   * Exports the Order class.
   * @param module The module to export to.
   */
  void ExportOrder(pybind11::module& module);

  /**
   * Exports the OrderCancellationReactor class.
   * @param module The module to export to.
   */
  void ExportOrderCancellationReactor(pybind11::module& module);

  /**
   * Exports the OrderExecutionService namespace.
   * @param module The module to export to.
   */
  void ExportOrderExecutionService(pybind11::module& module);

  /**
   * Exports the OrderExecutionServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportOrderExecutionServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the OrderFields struct.
   * @param module The module to export to.
   */
  void ExportOrderFields(pybind11::module& module);

  /**
   * Exports the OrderInfo struct.
   * @param module The module to export to.
   */
  void ExportOrderInfo(pybind11::module& module);

  /**
   * Exports the OrderReactor class.
   * @param module The module to export to.
   */
  void ExportOrderReactor(pybind11::module& module);

  /**
   * Exports the OrderRecord struct.
   * @param module The module to export to.
   */
  void ExportOrderRecord(pybind11::module& module);

  /**
   * Exports the OrderWrapperReactor class.
   * @param module The module to export to.
   */
  void ExportOrderWrapperReactor(pybind11::module& module);

  /**
   * Exports the PrimitiveOrder class.
   * @param module The module to export to.
   */
  void ExportPrimitiveOrder(pybind11::module& module);

  /**
   * Exports the standard queries.
   * @param module The module to export to.
   */
  void ExportStandardQueries(pybind11::module& module);

  /**
   * Exports the SqliteOrderExecutionDataStore class.
   * @param module The module to export to.
   */
  void ExportSqliteOrderExecutionDataStore(pybind11::module& module);

  /**
   * Exports an OrderExecutionClient class.
   * @param <Client> The type of OrderExecutionClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported OrderExecutionClient.
   */
  template<typename Client>
  auto ExportOrderExecutionClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_order", [] (Client& self, OrderExecutionService::OrderId id) ->
          const OrderExecutionService::Order* {
        auto order = self.LoadOrder(id);
        if(!order) {
          return nullptr;
        }
        return &*order;
      }, pybind11::return_value_policy::reference_internal).
      def("query_order_records", &Client::QueryOrderRecords).
      def("query_sequenced_order_submissions", static_cast<void (Client::*)(
        const OrderExecutionService::AccountQuery&,
        Beam::ScopedQueueWriter<OrderExecutionService::SequencedOrder>)>(
          &Client::QueryOrderSubmissions)).
      def("query_order_submissions", static_cast<void (Client::*)(
        const OrderExecutionService::AccountQuery&,
        Beam::ScopedQueueWriter<const OrderExecutionService::Order*>)>(
          &Client::QueryOrderSubmissions)).
      def("query_execution_reports", &Client::QueryExecutionReports).
      def("submit", &Client::Submit,
        pybind11::return_value_policy::reference_internal).
      def("cancel", &Client::Cancel).
      def("update", &Client::Update).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        OrderExecutionService::OrderExecutionClientBox>) {
      pybind11::implicitly_convertible<Client,
        OrderExecutionService::OrderExecutionClientBox>();
      GetExportedOrderExecutionClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }

  /**
   * Exports an OrderExecutionDataStore class.
   * @param <DataStore> The type of OrderExecutionDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported OrderExecutionDataStore.
   */
  template<typename DataStore>
  auto ExportOrderExecutionDataStore(pybind11::module& module,
      const std::string& name) {
    auto dataStore = pybind11::class_<DataStore, std::shared_ptr<DataStore>>(
      module, name.c_str()).
      def("load_order", &DataStore::LoadOrder).
      def("load_order_submissions", static_cast<
        std::vector<OrderExecutionService::SequencedOrderRecord> (
          DataStore::*)(const OrderExecutionService::AccountQuery&)>(
            &DataStore::LoadOrderSubmissions)).
      def("load_execution_reports", static_cast<
        std::vector<OrderExecutionService::SequencedExecutionReport> (
          DataStore::*)(const OrderExecutionService::AccountQuery&)>(
            &DataStore::LoadExecutionReports)).
      def("store", static_cast<void (DataStore::*)(
        const OrderExecutionService::SequencedAccountOrderInfo&)>(
          &DataStore::Store)).
      def("store", static_cast<void (DataStore::*)(
        const std::vector<OrderExecutionService::SequencedAccountOrderInfo>&)>(
          &DataStore::Store)).
      def("store", static_cast<void (DataStore::*)(
        const OrderExecutionService::SequencedAccountExecutionReport&)>(
          &DataStore::Store)).
      def("store", static_cast<void (DataStore::*)(
        const std::vector<
          OrderExecutionService::SequencedAccountExecutionReport>&)>(
            &DataStore::Store)).
      def("close", &DataStore::Close);
    if constexpr(!std::is_same_v<DataStore,
        OrderExecutionService::OrderExecutionDataStoreBox>) {
      pybind11::implicitly_convertible<DataStore,
        OrderExecutionService::OrderExecutionDataStoreBox>();
      GetExportedOrderExecutionDataStoreBox().def(
        pybind11::init<std::shared_ptr<DataStore>>());
    }
    return dataStore;
  }
}

#endif
