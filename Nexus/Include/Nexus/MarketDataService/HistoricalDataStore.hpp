#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Utilities/Concept.hpp>
#include <boost/optional.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/MarketEntry.hpp"
#include "Nexus/MarketDataService/QueryTypes.hpp"
#include "Nexus/MarketDataService/SecurityEntry.hpp"

namespace Nexus::MarketDataService {

  /** Concept used to store historical market data. */
  struct HistoricalDataStore : Beam::Concept<HistoricalDataStore> {

    /**
     * Loads SecurityInfo objects that match a query.
     * @param query The query to load.
     * @return The list of all SecurityInfo objects that match the <i>query</i>.
     */
    std::vector<SecurityInfo> LoadSecurityInfo(const SecurityInfoQuery& query);

    /**
     * Executes a search query over a Market's OrderImbalances.
     * @param query The search query to execute.
     * @return The list of the OrderImbalances that satisfy the search
     *         <i>query</i>.
     */
    std::vector<SequencedOrderImbalance> LoadOrderImbalances(
      const VenueQuery& query);

    /**
     * Executes a search query over a Security's BboQuotes.
     * @param query The search query to execute.
     * @return The list of BboQuotes that satisfy the search <i>query</i>.
     */
    std::vector<SequencedBboQuote> LoadBboQuotes(
      const SecurityMarketDataQuery& query);

    /**
     * Executes a search query over a Security's BookQuotes.
     * @param query The search query to execute.
     * @return The list of BookQuotes that satisfy the search <i>query</i>.
     */
    std::vector<SequencedBookQuote> LoadBookQuotes(
      const SecurityMarketDataQuery& query);

    /**
     * Executes a search query over a Security's TimeAndSales.
     * @param query The search query to execute.
     * @return The list of TimeAndSales that satisfy the search <i>query</i>.
     */
    std::vector<SequencedTimeAndSale> LoadTimeAndSales(
      const SecurityMarketDataQuery& query);

    /**
     * Stores a SecurityInfo.
     * @param info The SecurityInfo to store.
     */
    void Store(const SecurityInfo& info);

    /**
     * Stores a list of SequencedVenueOrderImbalances.
     * @param orderImbalances The SequencedVenueOrderImbalances to store.
     */
    void Store(const std::vector<SequencedVenueOrderImbalance>&
      orderImbalances);

    /**
     * Stores a SequencedSecurityBboQuote.
     * @param bboQuote The SequencedSecurityBboQuote to store.
     */
    void Store(const SequencedSecurityBboQuote& bboQuote);

    /**
     * Stores a list of SequencedSecurityBboQuotes.
     * @param bboQuotes The list of SequencedSecurityBboQuotes to store.
     */
    void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

    /**
     * Stores a SequencedSecurityBookQuote.
     * @param bookQuote The SequencedSecurityBookQuote to store.
     */
    void Store(const SequencedSecurityBookQuote& bookQuote);

    /**
     * Stores a list of SequencedSecurityBookQuotes.
     * @param bookQuotes The list of SequencedSecurityBookQuotes to store.
     */
    void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

    /**
     * Stores a SequencedSecurityTimeAndSale.
     * @param timeAndSale The SequencedSecurityTimeAndSale to store.
     */
    void Store(const SequencedSecurityTimeAndSale& timeAndSale);

    /**
     * Stores a list of SequencedSecurityTimeAndSales.
     * @param timeAndSales The list of SequencedSecurityTimeAndSales to store.
     */
    void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

    /** Closes the data store. */
    void Close();
  };
}

#endif
