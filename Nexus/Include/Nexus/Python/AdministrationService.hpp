#ifndef NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#define NEXUS_PYTHON_ADMINISTRATION_SERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the AccountIdentity struct.
  void ExportAccountIdentity();

  //! Exports the AccountModificationRequest class.
  void ExportAccountModificationRequest();

  //! Exports the AdministrationClient class.
  void ExportAdministrationClient();

  //! Exports the AdministrationService namespace.
  void ExportAdministrationService();

  //! Exports the AdministrationServiceTestEnvironment class.
  void ExportAdministrationServiceTestEnvironment();

  //! Exports the ApplicationAdministrationClient class.
  void ExportApplicationAdministrationClient();

  //! Exports the EntitlementModificationRequest class.
  void ExportEntitlementModification();

  //! Exports the Message class.
  void ExportMessage();

  //! Exports the TradingGroup class.
  void ExportTradingGroup();
}
}

#endif
