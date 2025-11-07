#ifndef NEXUS_PYTHON_CLIENTS_HPP
#define NEXUS_PYTHON_CLIENTS_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/Clients/Clients.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported Clients. */
  NEXUS_EXPORT_DLL pybind11::class_<Clients>& get_exported_clients();

  /**
   * Exports a Clients class.
   * @param <C> The type of Clients to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Clients.
   */
  template<typename C>
  auto export_clients(pybind11::module& module, std::string_view name) {
    auto clients = pybind11::class_<C>(module, name.data()).
      def_property_readonly(
        "service_locator_client", &C::get_service_locator_client,
        pybind11::return_value_policy::reference).
      def_property_readonly(
        "administration_client", &C::get_administration_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("definitions_client", &C::get_definitions_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("market_data_client", &C::get_market_data_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("charting_client", &C::get_charting_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("compliance_client", &C::get_compliance_client,
        pybind11::return_value_policy::reference).
      def_property_readonly(
        "order_execution_client", &C::get_order_execution_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("risk_client", &C::get_risk_client,
        pybind11::return_value_policy::reference).
      def_property_readonly("time_client", &C::get_time_client,
        pybind11::return_value_policy::reference).
      def("make_timer", &C::make_timer).
      def("close", &C::close);
    if constexpr(!std::is_same_v<C, Clients>) {
      pybind11::implicitly_convertible<C, Clients>();
      get_exported_clients().
        def(pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
    }
    return clients;
  }

  /**
   * Exports the ServiceClients class.
   * @param module The module to export to.
   */
  void export_service_clients(pybind11::module& module);
}

#endif
