#ifndef NEXUS_PYTHON_RISK_SERVICE_HPP
#define NEXUS_PYTHON_RISK_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/Python/DllExport.hpp"
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/RiskService/RiskDataStore.hpp"

namespace Nexus::Python {

  /** Returns the exported RiskClient. */
  NEXUS_EXPORT_DLL pybind11::class_<RiskClient>& get_exported_risk_client();

  /** Returns the exported RiskDataStore. */
  NEXUS_EXPORT_DLL pybind11::class_<RiskDataStore>&
    get_exported_risk_data_store();

  /**
   * Exports the InventorySnapshot class.
   * @param module The module to export to.
   */
  void export_inventory_snapshot(pybind11::module& module);

  /**
   * Exports the LocalRiskDataStore class.
   * @param module The module to export to.
   */
  void export_local_risk_data_store(pybind11::module& module);

  /**
   * Exports the SqlRiskDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void export_mysql_risk_data_store(pybind11::module& module);

  /**
   * Exports a RiskClient class.
   * @param <C> The type of RiskClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported RiskClient.
   */
  template<IsRiskClient C>
  auto export_risk_client(pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C>(module, name.data()).
      def("load_inventory_snapshot", &C::load_inventory_snapshot).
      def("reset", &C::reset).
      def_property_readonly("risk_portfolio_update_publisher",
        &C::get_risk_portfolio_update_publisher,
        pybind11::return_value_policy::reference).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, RiskClient>) {
      pybind11::implicitly_convertible<C, RiskClient>();
      get_exported_risk_client().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /**
   * Exports a RiskDataStore class.
   * @param <D> The type of RiskDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported RiskDataStore.
   */
  template<IsRiskDataStore D>
  auto export_risk_data_store(pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<D>(module, name.data()).
      def("load_inventory_snapshot", &D::load_inventory_snapshot).
      def("store", &D::store).
      def("close", &D::close);
    if constexpr(!std::is_same_v<D, RiskDataStore>) {
      pybind11::implicitly_convertible<D, RiskDataStore>();
      get_exported_risk_data_store().
        def(pybind11::init<D*>(), pybind11::keep_alive<1, 2>());
    }
    return data_store;
  }

  /**
   * Exports the RiskParameters class.
   * @param module The module to export to.
   */
  void export_risk_parameters(pybind11::module& module);

  /**
   * Exports the RiskService namespace.
   * @param module The module to export to.
   */
  void export_risk_service(pybind11::module& module);

  /**
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_risk_service_application_definitions(pybind11::module& module);

  /**
   * Exports the RiskServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_risk_service_test_environment(pybind11::module& module);

  /**
   * Exports the RiskState struct.
   * @param module The module to export to.
   */
  void export_risk_state(pybind11::module& module);

  /**
   * Exports the SqlRiskDataStore class connecting to SQLite.
   * @param module The module to export to.
   */
  void export_sqlite_risk_data_store(pybind11::module& module);
}

#endif
