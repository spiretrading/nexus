#ifndef NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#define NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /**
   * Returns the exported DefinitionsClient.
   * @return The exported DefinitionsClient.
   */
  NEXUS_EXPORT_DLL pybind11::class_<DefinitionsClient>&
    get_exported_definitions_client();

  /**
   * Exports the DefaultDefinitionsClient class.
   * @param module The module to export to.
   */
  void export_default_definitions_client(pybind11::module& module);

  /**
   * Exports the DefinitionsService namespace.
   * @param module The module to export to.
   */
  void export_definitions_service(pybind11::module& module);

  /**
   * Exports the application definitions.
   * @param module The module to export to.
   */
  void export_definitions_service_application_definitions(
    pybind11::module& module);

  /**
   * Exports the DefinitionsServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_definitions_service_test_environment(pybind11::module& module);

  /**
   * Exports a DefinitionsClient class.
   * @param <T> The type of DefinitionsClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported DefinitionsClient.
   */
  template<IsDefinitionsClient T>
  auto export_definitions_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<T>(module, name.data()).
      def("load_minimum_spire_client_version",
        &T::load_minimum_spire_client_version).
      def("load_organization_name", &T::load_organization_name).
      def("load_country_database", &T::load_country_database).
      def("load_time_zone_database", &T::load_time_zone_database).
      def("load_currency_database", &T::load_currency_database).
      def("load_destination_database", &T::load_destination_database).
      def("load_venue_database", &T::load_venue_database).
      def("load_exchange_rates", &T::load_exchange_rates).
      def("load_compliance_rule_schemas", &T::load_compliance_rule_schemas).
      def("load_trading_schedule", &T::load_trading_schedule).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, DefinitionsClient>) {
      pybind11::implicitly_convertible<T, DefinitionsClient>();
      get_exported_definitions_client().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }
}

#endif
