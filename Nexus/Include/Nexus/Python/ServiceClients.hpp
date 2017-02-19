#ifndef NEXUS_PYTHONSERVICECLIENTS_HPP
#define NEXUS_PYTHONSERVICECLIENTS_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the ApplicationServiceClients class.
  void ExportApplicationServiceClients();

  //! Exports all ServiceClients classes.
  void ExportServiceClients();

  //! Exports the TestEnvironment class.
  void ExportTestEnvironment();

  //! Exports the TestServiceClients class.
  void ExportTestServiceClients();

  //! Exports the TestTimeClient class.
  void ExportTestTimeClient();

  //! Exports the TestTimer class.
  void ExportTestTimer();

  //! Exports the VirtualServiceClients class.
  void ExportVirtualServiceClients();
}
}

#endif
