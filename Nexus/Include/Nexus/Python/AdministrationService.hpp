#ifndef NEXUS_PYTHONADMINISTRATIONSERVICE_HPP
#define NEXUS_PYTHONADMINISTRATIONSERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the AccountIdentity struct.
  void ExportAccountIdentity();

  //! Exports the AdministrationClient class.
  void ExportAdministrationClient();

  //! Exports the AdministrationService namespace.
  void ExportAdministrationService();

  //! Exports the AdministrationServiceTestInstance class.
  void ExportAdministrationServiceTestInstance();

  //! Exports the TradingGroup class.
  void ExportTradingGroup();
}
}

#endif
