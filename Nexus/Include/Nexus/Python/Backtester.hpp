#ifndef NEXUS_PYTHON_BACKTESTER_HPP
#define NEXUS_PYTHON_BACKTESTER_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ActiveBacktesterEvent class.
   * @param module The module to export to.
   */
  void export_active_backtester_event(pybind11::module& module);

  /**
   * Exports all of the backtester classes.
   * @param module The module to export to.
   */
  void export_backtester(pybind11::module& module);

  /**
   * Exports the BacktesterClients class.
   * @param module The module to export to.
   */
  void export_backtester_clients(pybind11::module& module);

  /**
   * Exports the BacktesterEnvironment class.
   * @param module The module to export to.
   */
  void export_backtester_environment(pybind11::module& module);

  /**
   * Exports the BacktesterEvent class.
   * @param module The module to export to.
   */
  void export_backtester_event(pybind11::module& module);

  /**
   * Exports the BacktesterEventHandler class.
   * @param module The module to export to.
   */
  void export_backtester_event_handler(pybind11::module& module);
}

#endif
