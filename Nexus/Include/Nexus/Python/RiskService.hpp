#ifndef NEXUS_PYTHON_RISK_SERVICE_HPP
#define NEXUS_PYTHON_RISK_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/RiskService/RiskClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported RiskClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<RiskService::RiskClientBox>&
    GetExportedRiskClientBox();

  /**
   * Exports the ApplicationRiskClient class.
   * @param module The module to export to.
   */
  void ExportApplicationRiskClient(pybind11::module& module);

  /**
   * Exports the InventorySnapshot class.
   * @param module The module to export to.
   */
  void ExportInventorySnapshot(pybind11::module& module);

  /**
   * Exports the LocalRiskDataStore class.
   * @param module The module to export to.
   */
  void ExportLocalRiskDataStore(pybind11::module& module);

  /**
   * Exports the SqlRiskDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void ExportMySqlRiskDataStore(pybind11::module& module);

  /**
   * Exports the RiskDataStore class.
   * @param module The module to export to.
   */
  void ExportRiskDataStore(pybind11::module& module);

  /**
   * Exports the RiskParameters class.
   * @param module The module to export to.
   */
  void ExportRiskParameters(pybind11::module& module);

  /**
   * Exports the RiskService namespace.
   * @param module The module to export to.
   */
  void ExportRiskService(pybind11::module& module);

  /**
   * Exports the RiskServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportRiskServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the RiskState struct.
   * @param module The module to export to.
   */
  void ExportRiskState(pybind11::module& module);

  /**
   * Exports the SqlRiskDataStore class connecting to SQLite.
   * @param module The module to export to.
   */
  void ExportSqliteRiskDataStore(pybind11::module& module);

  /**
   * Exports a RiskClient class.
   * @param <Client> The type of RiskClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported RiskClient.
   */
  template<typename Client>
  auto ExportRiskClient(pybind11::module& module, const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_inventory_snapshot", &Client::LoadInventorySnapshot).
      def("reset", &Client::Reset).
      def("get_risk_portfolio_update_publisher",
        &Client::GetRiskPortfolioUpdatePublisher).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, RiskService::RiskClientBox>) {
      pybind11::implicitly_convertible<Client, RiskService::RiskClientBox>();
      GetExportedRiskClientBox().def(pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
