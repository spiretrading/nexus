#ifndef NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#define NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the AccountIdentity struct.
   * @param module The module to export to.
   */
  void ExportAccountIdentity(pybind11::module& module);

  /**
   * Exports the AccountModificationRequest class.
   * @param module The module to export to.
   */
  void ExportAccountModificationRequest(pybind11::module& module);

  /**
   * Exports the AdministrationClient class.
   * @param module The module to export to.
   */
  void ExportAdministrationClient(pybind11::module& module);

  /**
   * Exports the AdministrationService namespace.
   * @param module The module to export to.
   */
  void ExportAdministrationService(pybind11::module& module);

  /**
   * Exports the AdministrationServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportAdministrationServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the ApplicationAdministrationClient class.
   * @param module The module to export to.
   */
  void ExportApplicationAdministrationClient(pybind11::module& module);

  /**
   * Exports the EntitlementModificationRequest class.
   * @param module The module to export to.
   */
  void ExportEntitlementModification(pybind11::module& module);

  /**
   * Exports the Message class.
   * @param module The module to export to.
   */
  void ExportMessage(pybind11::module& module);

  //! Exports the TradingGroup class.
  /**
   * Exports the TradingGroup class.
   * @param module The module to export to.
   */
  void ExportTradingGroup(pybind11::module& module);
}

#endif
