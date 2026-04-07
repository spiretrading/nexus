#ifndef NEXUS_PYTHON_FEE_HANDLING_HPP
#define NEXUS_PYTHON_FEE_HANDLING_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the AsxTradeMatchFeeTable class.
   * @param module The module to export to.
   */
  void export_asx_trade_match_fee_table(pybind11::module& module);

  /**
   * Exports the ChicFeeTable class.
   * @param module The module to export to.
   */
  void export_chic_fee_table(pybind11::module& module);

  /**
   * Exports the ConsolidatedTmxFeeTable class.
   * @param module The module to export to.
   */
  void export_consolidated_tmx_fee_table(pybind11::module& module);

  /**
   * Exports the CseFeeTable class.
   * @param module The module to export to.
   */
  void export_cse_fee_table(pybind11::module& module);

  /**
   * Exports the Cse2FeeTable class.
   * @param module The module to export to.
   */
  void export_cse2_fee_table(pybind11::module& module);

  /**
   * Exports the CxdFeeTable class.
   * @param module The module to export to.
   */
  void export_cxd_fee_table(pybind11::module& module);

  /**
   * Exports the FeeHandling classes.
   * @param module The module to export to.
   */
  void export_fee_handling(pybind11::module& module);

  /**
   * Exports the LiquidityFlag enum.
   * @param module The module to export to.
   */
  void export_liquidity_flag(pybind11::module& module);

  /**
   * Exports the LynxFeeTable class.
   * @param module The module to export to.
   */
  void export_lynx_fee_table(pybind11::module& module);

  /**
   * Exports the MatnFeeTable class.
   * @param module The module to export to.
   */
  void export_matn_fee_table(pybind11::module& module);

  /**
   * Exports the NeoeFeeTable class.
   * @param module The module to export to.
   */
  void export_neoe_fee_table(pybind11::module& module);

  /**
   * Exports the NsxFeeTable class.
   * @param module The module to export to.
   */
  void export_nex_fee_table(pybind11::module& module);

  /**
   * Exports the OmgaFeeTable class.
   * @param module The module to export to.
   */
  void export_omga_fee_table(pybind11::module& module);

  /**
   * Exports the ParseFeeTable translation unit.
   * @param module The module to export to.
   */
  void export_parse_fee_table(pybind11::module& module);

  /**
   * Exports the PureFeeTable class.
   * @param module The module to export to.
   */
  void export_pure_fee_table(pybind11::module& module);

  /**
   * Exports the TsxFeeTable class.
   * @param module The module to export to.
   */
  void export_tsx_fee_table(pybind11::module& module);

  /**
   * Exports the XatsFeeTable class.
   * @param module The module to export to.
   */
  void export_xats_fee_table(pybind11::module& module);

  /**
   * Exports the Xcx2FeeTable class.
   * @param module The module to export to.
   */
  void export_xcx2_fee_table(pybind11::module& module);
}

#endif
