#ifndef NEXUS_PYTHON_FEE_HANDLING_HPP
#define NEXUS_PYTHON_FEE_HANDLING_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the AsxtFeeTable class.
   * @param module The module to export to.
   */
  void ExportAsxtFeeTable(pybind11::module& module);

  /**
   * Exports the ChicFeeTable class.
   * @param module The module to export to.
   */
  void ExportChicFeeTable(pybind11::module& module);

  /**
   * Exports the ConsolidatedTmxFeeTable class.
   * @param module The module to export to.
   */
  void ExportConsolidatedTmxFeeTable(pybind11::module& module);

  /**
   * Exports the FeeHandling classes.
   * @param module The module to export to.
   */
  void ExportFeeHandling(pybind11::module& module);

  /**
   * Exports the HkexFeeTable class.
   * @param module The module to export to.
   */
  void ExportHkexFeeTable(pybind11::module& module);

  /**
   * Exports the JpxFeeTable class.
   * @param module The module to export to.
   */
  void ExportJpxFeeTable(pybind11::module& module);

  /**
   * Exports the LiquidityFlag enum.
   * @param module The module to export to.
   */
  void ExportLiquidityFlag(pybind11::module& module);

  /**
   * Exports the PureFeeTable class.
   * @param module The module to export to.
   */
  void ExportPureFeeTable(pybind11::module& module);
}

#endif
