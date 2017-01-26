#ifndef NEXUS_PYTHONSERVICECLIENTS_HPP
#define NEXUS_PYTHONSERVICECLIENTS_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the ApplicationServiceClients class.
  void ExportApplicationServiceClients();

  //! Exports all ServiceClients classes.
  void ExportServiceClients();

  //! Exports the VirtualServiceClients class.
  void ExportVirtualServiceClients();
}
}

#endif
