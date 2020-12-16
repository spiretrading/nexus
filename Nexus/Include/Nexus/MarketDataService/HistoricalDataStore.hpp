#ifndef NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_HISTORICAL_DATA_STORE_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/Utilities/Concept.hpp>
#include <boost/optional.hpp>
#include "Nexus/Definitions/Definitions.hpp"
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
      const MarketWideDataQuery& query);

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
     * Executes a search query over a Security's MarketQuotes.
     * @param query The search query to execute.
     * @return The list of MarketQuotes that satisfy the search <i>query</i>.
     */
    std::vector<SequencedMarketQuote> LoadMarketQuotes(
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
     * Stores a SequencedMarketOrderImbalance.
     * @param orderImbalance The SequencedMarketOrderImbalance to store.
     */
    void Store(const SequencedMarketOrderImbalance& orderImbalance);

    /**
     * Stores a list of SequencedMarketOrderImbalances.
     * @param orderImbalances The SequencedMarketOrderImbalances to store.
     */
    void Store(const std::vector<SequencedMarketOrderImbalance>&
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
     * Stores a SequencedSecurityMarketQuote.
     * @param marketQuote The SequencedSecurityMarketQuote to store.
     */
    void Store(const SequencedSecurityMarketQuote& marketQuote);

    /**
     * Stores a list of SequencedSecurityMarketQuotes.
     * @param marketQuotes The list of SequencedSecurityMarketQuotes to store.
     */
    void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

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
