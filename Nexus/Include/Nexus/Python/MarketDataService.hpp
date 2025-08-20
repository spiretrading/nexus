#ifndef NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#define NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Nexus::Python {

  /**
   * Exports the AsyncHistoricalDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void ExportAsyncHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the CachedHistoricalDataStore class connecting to MySQL.
   * @param module The module to export to.
   */
  void ExportCachedHistoricalDataStore(pybind11::module& module);

  /**
   * Exports the HistoricalDataStore class.
   * @param module The module to export to.
   */
  void ExportHistoricalDataStore(pybind11::module& module);

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

  /**
   * Exports an HistoricalDataStore class.
   * @param <D> The type of HistoricalDataStore to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported HistoricalDataStore.
   */
  template<typename D>
  auto ExportHistoricalDataStore(
    pybind11::module& module, std::string_view name) {}

  /**
   * Exports a MarketDataClient class.
   * @param <C> The type of MarketDataClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataClient.
   */
  template<typename C>
  auto ExportMarketDataClient(
      pybind11::module& module, std::string_view name) {
  }

  /**
   * Exports a MarketDataFeedClient class.
   * @param <C> The type of MarketDataFeedClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported MarketDataFeedClient.
   */
  template<typename C>
  auto ExportMarketDataFeedClient(
      pybind11::module& module, std::string_view name) {
  }
}

#endif
