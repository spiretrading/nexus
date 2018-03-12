#ifndef NEXUS_PYTHON_COMPLIANCE_HPP
#define NEXUS_PYTHON_COMPLIANCE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the ApplicationComplianceClient class.
  void ExportApplicationComplianceClient();

  //! Exports the ComplianceClient class.
  void ExportComplianceClient();

  //! Exports the Compliance namespace.
  void ExportCompliance();

  //! Exports the ComplianceParameter struct.
  void ExportComplianceParameter();

  //! Exports the ComplianceRuleEntry class.
  void ExportComplianceRuleEntry();

  //! Exports the ComplianceRuleSchema class.
  void ExportComplianceRuleSchema();

  //! Exports the ComplianceRuleViolationRecord struct.
  void ExportComplianceRuleViolationRecord();
}
}

#endif
