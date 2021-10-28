#ifndef NEXUS_PYTHON_TELEMETRY_SERVICE_HPP
#define NEXUS_PYTHON_TELEMETRY_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/TelemetryService/TelemetryClientBox.hpp"
#include "Nexus/TelemetryService/TelemetryDataStoreBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported TelemetryClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<TelemetryService::TelemetryClientBox>&
    GetExportedTelemetryClientBox();

  /** Returns the exported TelemetryDataStoreBox. */
  NEXUS_EXPORT_DLL pybind11::class_<TelemetryService::TelemetryDataStoreBox>&
    GetExportedTelemetryDataStoreBox();

  /**
   * Exports the ApplicationTelemetryClient class.
   * @param module The module to export to.
   */
  void ExportApplicationTelemetryClient(pybind11::module& module);

  /**
   * Exports the LocalTelemetryDataStore class.
   * @param module The module to export to.
   */
  void ExportLocalTelemetryDataStore(pybind11::module& module);

  /**
   * Exports the MySqlTelemetryDataStore class.
   * @param module The module to export to.
   */
  void ExportMySqlTelemetryDataStore(pybind11::module& module);

  /**
   * Exports the SqliteTelemetryDataStore class.
   * @param module The module to export to.
   */
  void ExportSqliteTelemetryDataStore(pybind11::module& module);

  /**
   * Exports the TelemetryEvent struct.
   * @param module The module to export to.
   */
  void ExportTelemetryEvent(pybind11::module& module);

  /**
   * Exports the TelemetryService namespace.
   * @param module The module to export to.
   */
  void ExportTelemetryService(pybind11::module& module);

  /**
   * Exports the TelemetryServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportTelemetryServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports a TelemetryClient class.
   * @param <Client> The type of TelemetryClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported TelemetryClient.
   */
  template<typename Client>
  auto ExportTelemetryClient(
      pybind11::module& module, const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("query_sequenced_telemetry_events", static_cast<void (Client::*)(
        const TelemetryService::AccountQuery&,
        Beam::ScopedQueueWriter<TelemetryService::SequencedTelemetryEvent>)>(
          &Client::QueryTelemetryEvents)).
      def("query_telemetry_events", static_cast<void (Client::*)(
        const TelemetryService::AccountQuery&,
        Beam::ScopedQueueWriter<TelemetryService::TelemetryEvent>)>(
          &Client::QueryTelemetryEvents)).
      def("record", &Client::Record).
      def("close", &Client::Close);
    if constexpr(
        !std::is_same_v<Client, TelemetryService::TelemetryClientBox>) {
      pybind11::implicitly_convertible<
        Client, TelemetryService::TelemetryClientBox>();
      GetExportedTelemetryClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }

  /**
   * Exports a TelemetryDataStore class.
   * @param <DataStore> The type of TelemetryDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported TelemetryDataStore.
   */
  template<typename DataStore>
  auto ExportTelemetryDataStore(
      pybind11::module& module, const std::string& name) {
    auto dataStore = pybind11::class_<DataStore, std::shared_ptr<DataStore>>(
      module, name.c_str()).
      def("load_telemetry_events", static_cast<
        std::vector<TelemetryService::SequencedTelemetryEvent> (
          DataStore::*)(const TelemetryService::AccountQuery&)>(
            &DataStore::LoadTelemetryEvents)).
      def("store", static_cast<void (DataStore::*)(
        const TelemetryService::SequencedAccountTelemetryEvent&)>(
          &DataStore::Store)).
      def("store", static_cast<void (DataStore::*)(
        const std::vector<TelemetryService::SequencedAccountTelemetryEvent>&)>(
          &DataStore::Store)).
      def("close", &DataStore::Close);
    if constexpr(
        !std::is_same_v<DataStore, TelemetryService::TelemetryDataStoreBox>) {
      pybind11::implicitly_convertible<DataStore,
        TelemetryService::TelemetryDataStoreBox>();
      GetExportedTelemetryDataStoreBox().def(
        pybind11::init<std::shared_ptr<DataStore>>());
    }
    return dataStore;
  }
}

#endif
