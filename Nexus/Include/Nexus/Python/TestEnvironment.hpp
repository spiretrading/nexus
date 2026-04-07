#ifndef NEXUS_PYTHON_TEST_ENVIRONMENT_HPP
#define NEXUS_PYTHON_TEST_ENVIRONMENT_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the TestClients class.
   * @param module The module to export to.
   */
  void export_test_clients(pybind11::module& module);

  /**
   * Exports all TestEnvironment classes.
   * @param module The module to export to.
   */
  void export_test_environment(pybind11::module& module);

  /**
   * Exports the TestEnvironmentException class.
   * @param module The module to export to.
   */
  void export_test_environment_exception(pybind11::module& module);
}

#endif
