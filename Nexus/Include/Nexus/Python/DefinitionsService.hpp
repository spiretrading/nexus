#ifndef NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#define NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/DefinitionsService/DefinitionsClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported DefinitionsClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<DefinitionsService::DefinitionsClientBox>&
    GetExportedDefinitionsClientBox();

  /**
   * Exports the ApplicationDefinitionsClient class.
   * @param module The module to export to.
   */
  void ExportApplicationDefinitionsClient(pybind11::module& module);

  /**
   * Exports the DefinitionsService namespace.
   * @param module The module to export to.
   */
  void ExportDefinitionsService(pybind11::module& module);

  /**
   * Exports the DefinitionsTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportDefinitionsServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports a DefinitionsClient class.
   * @param <Client> The type of DefinitionsClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported DefinitionsClient.
   */
  template<typename Client>
  auto ExportDefinitionsClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_minimum_spire_client_version",
        &Client::LoadMinimumSpireClientVersion).
      def("load_organization_name", &Client::LoadOrganizationName).
      def("load_country_database", &Client::LoadCountryDatabase).
      def("load_time_zone_database", &Client::LoadTimeZoneDatabase).
      def("load_currency_database", &Client::LoadCurrencyDatabase).
      def("load_destination_database", &Client::LoadDestinationDatabase).
      def("load_market_database", &Client::LoadMarketDatabase).
      def("load_exchange_rates", &Client::LoadExchangeRates).
      def("load_compliance_rule_schemas", &Client::LoadComplianceRuleSchemas).
      def("load_trading_schedule", &Client::LoadTradingSchedule).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        DefinitionsService::DefinitionsClientBox>) {
      pybind11::implicitly_convertible<Client,
        DefinitionsService::DefinitionsClientBox>();
      GetExportedDefinitionsClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
