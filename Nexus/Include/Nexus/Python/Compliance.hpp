#ifndef NEXUS_PYTHON_COMPLIANCE_HPP
#define NEXUS_PYTHON_COMPLIANCE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ApplicationComplianceClient class.
   * @param module The module to export to.
   */
  void ExportApplicationComplianceClient(pybind11::module& module);

  /**
   * Exports the ComplianceClient class.
   * @param module The module to export to.
   */
  void ExportComplianceClient(pybind11::module& module);

  /**
   * Exports the Compliance namespace.
   * @param module The module to export to.
   */
  void ExportCompliance(pybind11::module& module);

  /**
   * Exports the ComplianceParameter struct.
   * @param module The module to export to.
   */
  void ExportComplianceParameter(pybind11::module& module);

  /**
   * Exports the ComplianceRuleEntry class.
   * @param module The module to export to.
   */
  void ExportComplianceRuleEntry(pybind11::module& module);

  /**
   * Exports the ComplianceRuleSchema class.
   * @param module The module to export to.
   */
  void ExportComplianceRuleSchema(pybind11::module& module);

  /**
   * Exports the ComplianceRuleViolationRecord struct.
   * @param module The module to export to.
   */
  void ExportComplianceRuleViolationRecord(pybind11::module& module);
}

#endif
