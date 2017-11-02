#ifndef NEXUS_PYTHONDEFINITIONSSERVICE_HPP
#define NEXUS_PYTHONDEFINITIONSSERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the ApplicationDefinitionsClient class.
  void ExportApplicationDefinitionsClient();

  //! Exports the DefinitionsClient class.
  void ExportDefinitionsClient();

  //! Exports the DefinitionsService namespace.
  void ExportDefinitionsService();
}
}

#endif
