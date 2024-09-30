#ifndef NEXUS_PYTHON_QUERIES_HPP
#define NEXUS_PYTHON_QUERIES_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the DataType classes.
   * @param module The module to export to.
   */
  void ExportDataType(pybind11::module& module);

  /**
   * Exports the Queries namespace.
   * @param module The module to export to.
   */
  void ExportQueries(pybind11::module& module);

  /**
   * Exports the TimeAndSaleAccessor class.
   * @param module The module to export to.
   */
  void ExportTimeAndSaleAccessor(pybind11::module& module);

  /**
   * Exports the Value classes.
   * @param module The module to export to.
   */
  void ExportValue(pybind11::module& module);
}

#endif
