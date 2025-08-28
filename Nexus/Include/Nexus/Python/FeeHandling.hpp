#ifndef NEXUS_PYTHON_FEE_HANDLING_HPP
#define NEXUS_PYTHON_FEE_HANDLING_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the AmexFeeTable class.
   * @param module The module to export to.
   */
  void ExportAmexFeeTable(pybind11::module& module);

  /**
   * Exports the ArcaFeeTable class.
   * @param module The module to export to.
   */
  void ExportArcaFeeTable(pybind11::module& module);

  /**
   * Exports the AsxtFeeTable class.
   * @param module The module to export to.
   */
  void ExportAsxtFeeTable(pybind11::module& module);

  /**
   * Exports the BatsFeeTable class.
   * @param module The module to export to.
   */
  void ExportBatsFeeTable(pybind11::module& module);

  /**
   * Exports the BatyFeeTable class.
   * @param module The module to export to.
   */
  void ExportBatyFeeTable(pybind11::module& module);

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
   * Exports the ConsolidatedUsFeeTable class.
   * @param module The module to export to.
   */
  void ExportConsolidatedUsFeeTable(pybind11::module& module);

  /**
   * Exports the CseFeeTable class.
   * @param module The module to export to.
   */
  void ExportCseFeeTable(pybind11::module& module);

  /**
   * Exports the Cse2FeeTable class.
   * @param module The module to export to.
   */
  void ExportCse2FeeTable(pybind11::module& module);

  /**
   * Exports the CxdFeeTable class.
   * @param module The module to export to.
   */
  void ExportCxdFeeTable(pybind11::module& module);

  /**
   * Exports the EdgaFeeTable class.
   * @param module The module to export to.
   */
  void ExportEdgaFeeTable(pybind11::module& module);

  /**
   * Exports the EdgxFeeTable class.
   * @param module The module to export to.
   */
  void ExportEdgxFeeTable(pybind11::module& module);

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
   * Exports the LynxFeeTable class.
   * @param module The module to export to.
   */
  void ExportLynxFeeTable(pybind11::module& module);

  /**
   * Exports the MatnFeeTable class.
   * @param module The module to export to.
   */
  void ExportMatnFeeTable(pybind11::module& module);

  /**
   * Exports the NeoeFeeTable class.
   * @param module The module to export to.
   */
  void ExportNeoeFeeTable(pybind11::module& module);

  /**
   * Exports the NsxFeeTable class.
   * @param module The module to export to.
   */
  void ExportNexFeeTable(pybind11::module& module);

  /**
   * Exports the NsdqFeeTable class.
   * @param module The module to export to.
   */
  void ExportNsdqFeeTable(pybind11::module& module);

  /**
   * Exports the NyseFeeTable class.
   * @param module The module to export to.
   */
  void ExportNyseFeeTable(pybind11::module& module);

  /**
   * Exports the OmgaFeeTable class.
   * @param module The module to export to.
   */
  void ExportOmgaFeeTable(pybind11::module& module);

  /**
   * Exports the PureFeeTable class.
   * @param module The module to export to.
   */
  void ExportPureFeeTable(pybind11::module& module);

  /**
   * Exports the TsxFeeTable class.
   * @param module The module to export to.
   */
  void ExportTsxFeeTable(pybind11::module& module);

  /**
   * Exports the XatsFeeTable class.
   * @param module The module to export to.
   */
  void ExportXatsFeeTable(pybind11::module& module);

  /**
   * Exports the Xcx2FeeTable class.
   * @param module The module to export to.
   */
  void ExportXcx2FeeTable(pybind11::module& module);
}

#endif
