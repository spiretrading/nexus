#ifndef NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#define NEXUS_PYTHON_DEFINITIONS_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ApplicationDefinitionsClient class.
   * @param module The module to export to.
   */
  void ExportApplicationDefinitionsClient(pybind11::module& module);

  /**
   * Exports the DefinitionsClient class.
   * @param module The module to export to.
   */
  void ExportDefinitionsClient(pybind11::module& module);

  /**
   * Exports the DefinitionsService namespace.
   * @param module The module to export to.
   */
  void ExportDefinitionsService(pybind11::module& module);
}

#endif
