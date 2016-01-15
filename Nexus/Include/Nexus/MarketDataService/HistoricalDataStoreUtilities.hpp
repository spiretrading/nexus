#ifndef NEXUS_MARKETDATAHISTORICALDATASTOREUTILITIES_HPP
#define NEXUS_MARKETDATAHISTORICALDATASTOREUTILITIES_HPP
#include <vector>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"

namespace Nexus {
namespace MarketDataService {
namespace Details {
  template<typename MarketDataType, typename HistoricalDataStore>
  struct HistoricalDataStoreLoader {};

  template<typename HistoricalDataStore>
  struct HistoricalDataStoreLoader<SequencedTimeAndSale, HistoricalDataStore> {
    std::vector<SequencedTimeAndSale> operator ()(
        HistoricalDataStore& dataStore,
        const SecurityMarketDataQuery& query) const {
      return dataStore.LoadTimeAndSales(query);
    }
  };
}

  //! Loads values from a HistoricalDataStore.
  /*!
    \param dataStore The HistoricalDataStore to load from.
    \param query The query to submit to the <i>dataStore</i>.
  */
  template<typename MarketDataType, typename HistoricalDataStore>
  std::vector<MarketDataType> HistoricalDataStoreLoad(
      HistoricalDataStore& dataStore,
      const typename MarketDataQueryType<MarketDataType>::type& query) {
    return Details::HistoricalDataStoreLoader<MarketDataType,
      HistoricalDataStore>()(dataStore, query);
  }
}
}

#endif
