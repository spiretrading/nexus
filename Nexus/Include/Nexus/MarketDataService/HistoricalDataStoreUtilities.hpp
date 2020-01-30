#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_UTILITIES_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_UTILITIES_HPP
#include <vector>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"

namespace Nexus::MarketDataService {
namespace Details {
  template<typename T, typename D>
  struct HistoricalDataStoreLoader {};

  template<typename D>
  struct HistoricalDataStoreLoader<SequencedOrderImbalance, D> {
    std::vector<SequencedOrderImbalance> operator ()(D& dataStore,
        const MarketWideDataQuery& query) const {
      return dataStore.LoadOrderImbalances(query);
    }
  };

  template<typename D>
  struct HistoricalDataStoreLoader<SequencedBboQuote, D> {
    std::vector<SequencedBboQuote> operator ()(D& dataStore,
        const SecurityMarketDataQuery& query) const {
      return dataStore.LoadBboQuotes(query);
    }
  };

  template<typename D>
  struct HistoricalDataStoreLoader<SequencedMarketQuote, D> {
    std::vector<SequencedMarketQuote> operator ()(D& dataStore,
        const SecurityMarketDataQuery& query) const {
      return dataStore.LoadMarketQuotes(query);
    }
  };

  template<typename D>
  struct HistoricalDataStoreLoader<SequencedBookQuote, D> {
    std::vector<SequencedBookQuote> operator ()(D& dataStore,
        const SecurityMarketDataQuery& query) const {
      return dataStore.LoadBookQuotes(query);
    }
  };

  template<typename D>
  struct HistoricalDataStoreLoader<SequencedTimeAndSale, D> {
    std::vector<SequencedTimeAndSale> operator ()(D& dataStore,
        const SecurityMarketDataQuery& query) const {
      return dataStore.LoadTimeAndSales(query);
    }
  };
}

  /**
   * Loads values from a HistoricalDataStore.
   * @param dataStore The HistoricalDataStore to load from.
   * @param query The query to submit to the <i>dataStore</i>.
   */
  template<typename MarketDataType, typename HistoricalDataStore>
  std::vector<MarketDataType> HistoricalDataStoreLoad(
      HistoricalDataStore& dataStore,
      const typename MarketDataQueryType<MarketDataType>::type& query) {
    return Details::HistoricalDataStoreLoader<MarketDataType,
      HistoricalDataStore>()(dataStore, query);
  }
}

#endif
