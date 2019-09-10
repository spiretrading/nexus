#ifndef NEXUS_PYTHON_BACKTESTER_HPP
#define NEXUS_PYTHON_BACKTESTER_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports all of the backtester classes.
   * @param module The module to export to.
   */
  void ExportBacktester(pybind11::module& module);

  /**
   * Exports the BacktesterEnvironment class.
   * @param module The module to export to.
   */
  void ExportBacktesterEnvironment(pybind11::module& module);

  /**
   * Exports the BacktesterEventHandler class.
   * @param module The module to export to.
   */
  void ExportBacktesterEventHandler(pybind11::module& module);

  /**
   * Exports the BacktesterServiceClients class.
   * @param module The module to export to.
   */
  void ExportBacktesterServiceClients(pybind11::module& module);
}

#endif
