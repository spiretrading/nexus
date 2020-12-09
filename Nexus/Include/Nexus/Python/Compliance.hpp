#ifndef NEXUS_PYTHON_COMPLIANCE_HPP
#define NEXUS_PYTHON_COMPLIANCE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Nexus/Compliance/ComplianceClientBox.hpp"
#include "Nexus/Python/DllExport.hpp"

namespace Nexus::Python {

  /** Returns the exported ComplianceClientBox. */
  NEXUS_EXPORT_DLL pybind11::class_<Compliance::ComplianceClientBox>&
    GetExportedComplianceClientBox();

  /**
   * Exports the ApplicationComplianceClient class.
   * @param module The module to export to.
   */
  void ExportApplicationComplianceClient(pybind11::module& module);

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

  /**
   * Exports the ComplianceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportComplianceTestEnvironment(pybind11::module& module);

  /**
   * Exports a ComplianceClient class.
   * @param <Client> The type of ComplianceClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ComplianceClient.
   */
  template<typename Client>
  auto ExportComplianceClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load", &Client::Load).
      def("add", &Client::Add).
      def("update", &Client::Update).
      def("delete", &Client::Delete).
      def("report", &Client::Report).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, Compliance::ComplianceClientBox>) {
      pybind11::implicitly_convertible<Client,
        Compliance::ComplianceClientBox>();
      GetExportedComplianceClientBox().def(pybind11::init<
        std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
