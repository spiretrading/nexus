#ifndef NEXUS_PYTHONSERVICECLIENTS_HPP
#define NEXUS_PYTHONSERVICECLIENTS_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the ApplicationServiceClients class.
  void ExportApplicationServiceClients();

  //! Exports all ServiceClients classes.
  void ExportServiceClients();

  //! Exports the TestServiceClientsInstance class.
  void ExportTestServiceClientsInstance();

  //! Exports the TestServiceClients class.
  void ExportTestServiceClients();

  //! Exports the VirtualServiceClients class.
  void ExportVirtualServiceClients();
}
}

#endif
