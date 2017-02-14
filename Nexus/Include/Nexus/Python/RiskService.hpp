#ifndef NEXUS_PYTHONRISKSERVICE_HPP
#define NEXUS_PYTHONRISKSERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus {
namespace Python {

  //! Exports the RiskService namespace.
  void ExportRiskService();

  //! Exports the RiskParameters class.
  void ExportRiskParameters();
}
}

#endif
