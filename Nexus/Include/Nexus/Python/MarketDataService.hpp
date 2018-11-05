#ifndef NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#define NEXUS_PYTHON_MARKET_DATA_SERVICE_HPP
#include "Nexus/Python/Python.hpp"

namespace Nexus::Python {

  //! Exports the ApplicationMarketDataClient class.
  void ExportApplicationMarketDataClient();

  //! Exports the HistoricalDataStore class.
  void ExportHistoricalDataStore();

  //! Exports the MarketDataClient class.
  void ExportMarketDataClient();

  //! Exports the MarketDataService namespace.
  void ExportMarketDataService();

  //! Exports the MarketDataServiceTestEnvironment class.
  void ExportMarketDataServiceTestEnvironment();

  //! Exports the SqlHistoricalDataStore class connecting to MySQL.
  void ExportMySqlHistoricalDataStore();

  //! Exports the SecuritySnapshot class.
  void ExportSecuritySnapshot();

  //! Exports the SqlHistoricalDataStore class connecting to SQLite.
  void ExportSqliteHistoricalDataStore();
}

#endif
