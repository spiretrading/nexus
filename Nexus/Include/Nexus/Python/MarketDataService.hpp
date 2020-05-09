#ifndef NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#define NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the ApplicationMarketDataClient class.
   * @param module The module to export to.
   */
  void ExportApplicationMarketDataClient(pybind11::module& module);

  /**
   * Exports the ApplicationMarketDataFeedClient class.
   * @param module The module to export to.
   */
  void ExportApplicationMarketDataFeedClient(pybind11::module& module);

  /**
   * Exports the HistoricalDataStore class.
   * @param module The module to export to.
   */
  void ExportHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the MarketDataClient class.
   * @param module The module to export to.
   */
  void ExportMarketDataClient(pybind11::module& module);

  /**
   * Exports the MarketDataFeedClient class.
   * @param module The module to export to.
   */
  void ExportMarketDataFeedClient(pybind11::module& module);

  /**
   * Exports the market data reactors.
   * @param module The module to export to.
   */
  void ExportMarketDataReactors(pybind11::module& module);

  /**
   * Exports the MarketDataService namespace.
   * @param module The module to export to.
   */
  void ExportMarketDataService(pybind11::module& module);

  /**
   * Exports the MarketDataServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportMarketDataServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void ExportMySqlHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the SecuritySnapshot class.
   * @param module The module to export to.
   */
  void ExportSecuritySnapshot(pybind11::module& module);

  /**
   * Exports the SqlHistoricalDataStore class connecting to SQLite.
   * @param module The module to export to.
   */
  void ExportSqliteHistoricalDataStore(pybind11::module& module);
}

#endif
