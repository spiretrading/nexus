#ifndef NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#define NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#include <string_view>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"

namespace Nexus::Python {

  /**
   * Exports the DefinitionsService namespace.
   * @param module The module to export to.
   */
  void export_definitions_service(pybind11::module& module);

  /**
   * Exports the DefinitionsServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void export_definitions_service_test_environment(pybind11::module& module);

  /**
   * Exports a DefinitionsClient class.
   * @param <C> The type of DefinitionsClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported DefinitionsClient.
   */
  template<DefinitionsService::IsDefinitionsClient C>
  auto export_definitions_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<C, std::shared_ptr<C>>(module, name.data()).
      def("load_minimum_spire_client_version",
        &C::load_minimum_spire_client_version).
      def("load_organization_name", &C::load_organization_name).
      def("load_country_database", &C::load_country_database).
      def("load_time_zone_database", &C::load_time_zone_database).
      def("load_currency_database", &C::load_currency_database).
      def("load_destination_database", &C::load_destination_database).
      def("load_venue_database", &C::load_venue_database).
      def("load_exchange_rates", &C::load_exchange_rates).
      def("load_compliance_rule_schemas", &C::load_compliance_rule_schemas).
      def("load_trading_schedule", &C::load_trading_schedule).
      def("close", &C::close);
    return client;
  }
}

#endif
