#ifndef NEXUS_PYTHON_RISK_SERVICE_HPP
#define NEXUS_PYTHON_RISK_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

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
}

#endif
