#ifndef NEXUS_PYTHON_ACCOUNTING_HPP
#define NEXUS_PYTHON_ACCOUNTING_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the Accounting namespace.
   * @param module The module to export to.
   */
  void ExportAccounting(pybind11::module& module);

  /**
   * Exports the BuyingPowerModel class.
   * @param module The module to export to.
   */
  void ExportBuyingPowerModel(pybind11::module& module);

  /**
   * Exports the PortfolioUpdateEntry class.
   * @param module The module to export to.
   */
  void ExportPortfolioUpdateEntry(pybind11::module& module);

  /**
   * Exports the PositionOrderBook class.
   * @param module The module to export to.
   */
  void ExportPositionOrderBook(pybind11::module& module);

  /**
   * Exports the Position<Security> class.
   * @param module The module to export to.
   */
  void ExportPosition(pybind11::module& module);

  /**
   * Exports the Inventory<Position<Security>> class.
   * @param module The module to export to.
   */
  void ExportSecurityInventory(pybind11::module& module);

  /**
   * Exports the SecurityValuation class.
   * @param module The module to export to.
   */
  void ExportSecurityValuation(pybind11::module& module);

  /**
   * Exports the TrueAverageBookkeeper class.
   * @param module The module to export to.
   */
  void ExportTrueAverageBookkeeper(pybind11::module& module);

  /**
   * Exports the BookkeeperReactor using a TrueAverageBookkeeper.
   * @param module The module to export to.
   */
  void ExportTrueAverageBookkeeperReactor(pybind11::module& module);

  /**
   * Exports the TrueAveragePortfolio class.
   * @param module The module to export to.
   */
  void ExportTrueAveragePortfolio(pybind11::module& module);
}

#endif
