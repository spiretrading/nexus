#ifndef NEXUS_PYTHON_RISK_SERVICE_HPP
#define NEXUS_PYTHON_RISK_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ApplicationRiskClient class.
   * @param module The module to export to.
   */
  void ExportApplicationRiskClient(pybind11::module& module);

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
   * Exports the RiskClient class.
   * @param module The module to export to.
   */
  void ExportRiskClient(pybind11::module& module);

  /**
   * Exports the RiskDataStore class.
   * @param module The module to export to.
   */
  void ExportRiskDataStore(pybind11::module& module);

  /**
   * Exports the RiskService namespace.
   * @param module The module to export to.
   */
  void ExportRiskService(pybind11::module& module);

  /**
   * Exports the RiskParameters class.
   * @param module The module to export to.
   */
  void ExportRiskParameters(pybind11::module& module);

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
}

#endif
