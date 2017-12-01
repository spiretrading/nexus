#ifndef NEXUS_PYTHONADMINISTRATIONSERVICE_HPP
#define NEXUS_PYTHONADMINISTRATIONSERVICE_HPP
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

  //! Exports the EntitlementModificationRequest class.
  void ExportEntitlementModificationRequest();

  //! Exports the Message class.
  void ExportMessage();

  //! Exports the TradingGroup class.
  void ExportTradingGroup();
}
}

#endif
