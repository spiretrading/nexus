#ifndef NEXUS_PYTHON_STAMP_HPP
#define NEXUS_PYTHON_STAMP_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports STAMP fields and messages.
   * @param module The module to export to.
   */
  void export_stamp(pybind11::module& module);

  /**
   * Exports a STAMP field.
   * @param module The module to export to.
   */
  void export_stamp_field(pybind11::module& module);

  /**
   * Exports a STAMP message and its sections.
   * @param module The module to export to.
   */
  void export_stamp_message(pybind11::module& module);
}

#endif
