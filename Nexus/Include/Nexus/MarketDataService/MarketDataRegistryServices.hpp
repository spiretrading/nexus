#ifndef NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"

namespace Nexus::MarketDataService {
  using OrderImbalanceQueryResult =
    Beam::Queries::QueryResult<SequencedOrderImbalance>;
  using BboQuoteQueryResult = Beam::Queries::QueryResult<SequencedBboQuote>;
  using BookQuoteQueryResult = Beam::Queries::QueryResult<SequencedBookQuote>;
  using MarketQuoteQueryResult =
    Beam::Queries::QueryResult<SequencedMarketQuote>;
  using TimeAndSaleQueryResult =
    Beam::Queries::QueryResult<SequencedTimeAndSale>;

  BEAM_DEFINE_SERVICES(MarketDataRegistryServices,

    /**
     * Queries a Market's OrderImbalances.
     * @param query The query to run.
     * @return The list of OrderImbalances satisfying the query.
     */
    (QueryOrderImbalancesService,
      "Nexus.MarketDataService.QueryOrderImbalancesService",
      OrderImbalanceQueryResult, MarketWideDataQuery, query),

    /**
     * Queries a Security's BboQuotes.
     * @param query The query to run.
     * @return The list of BboQuotes satisfying the query.
     */
    (QueryBboQuotesService, "Nexus.MarketDataService.QueryBboQuotesService",
      BboQuoteQueryResult, SecurityMarketDataQuery, query),

    /**
     * Queries a Security's BookQuotes.
     * @param query The query to run.
     * @return The list of BookQuotes satisfying the query.
     */
    (QueryBookQuotesService, "Nexus.MarketDataService.QueryBookQuotesService",
      BookQuoteQueryResult, SecurityMarketDataQuery, query),

    /**
     * Queries a Security's MarketQuotes.
     * @param query The query to run.
     * @return The list of MarketQuotes satisfying the query.
     */
    (QueryMarketQuotesService,
      "Nexus.MarketDataService.QueryMarketQuotesService",
      MarketQuoteQueryResult, SecurityMarketDataQuery, query),

    /**
     * Queries a Security's TimeAndSales.
     * @param query The query to run.
     * @return The list of TimeAndSales satisfying the query.
     */
    (QueryTimeAndSalesService,
      "Nexus.MarketDataService.QueryTimeAndSalesService",
      TimeAndSaleQueryResult, SecurityMarketDataQuery, query),

    /**
     * Loads a Security's real-time snapshot.
     * @param security The Security whose snapshot is to be loaded.
     * @return The SecuritySnapshot for the specified <i>security</i>.
     */
    (LoadSecuritySnapshotService,
      "Nexus.MarketDataService.LoadSecuritySnapshotService", SecuritySnapshot,
      Security, security),

    /**
     * Loads the SecurityTechnicals for a specified Security.
     * @param security The Security whose SecurityTechnicals is to be loaded.
     * @return The SecurityTechnicals for the specified <i>security</i>.
     */
    (LoadSecurityTechnicalsService,
      "Nexus.MarketDataService.LoadSecurityTechnicalsService",
      SecurityTechnicals, Security, security),

    /**
     * Queries for all SecurityInfo objects that are within a region.
     * @param security The Security whose SecurityInfo is to be loaded.
     * @return The list of SecurityInfo objects that match the <i>query</i>.
     */
    (QuerySecurityInfoService,
      "Nexus.MarketDataService.QuerySecurityInfoService",
      std::vector<SecurityInfo>, SecurityInfoQuery, query),

    /**
     * Loads all SecurityInfo objects that match a prefix.
     * @param prefix The prefix to search for.
     * @return The list of SecurityInfo objects that match the <i>prefix</i>.
     */
    (LoadSecurityInfoFromPrefixService,
      "Nexus.MarketDataService.LoadSecurityInfoFromPrefixService",
      std::vector<SecurityInfo>, std::string, prefix));

  BEAM_DEFINE_MESSAGES(MarketDataRegistryMessages,

    /**
     * Sends a query's SequencedMarketOrderImbalance.
     * @param order_imbalance The query's SequencedMarketOrderImbalance.
     */
    (OrderImbalanceMessage, "Nexus.MarketDataService.OrderImbalanceMessage",
      SequencedMarketOrderImbalance, order_imbalance),

    /**
     * Sends a query's SequencedSecurityBboQuote.
     * @param bbo_quote The query's SequencedSecurityBboQuote.
     */
    (BboQuoteMessage, "Nexus.MarketDataService.BboQuoteMessage",
      SequencedSecurityBboQuote, bbo_quote),

    /**
     * Sends a query's SequencedSecurityMarketQuote.
     * @param market_quote The query's SequencedSecurityMarketQuote.
     */
    (MarketQuoteMessage, "Nexus.MarketDataService.MarketQuoteMessage",
      SequencedSecurityMarketQuote, market_quote),

    /**
     * Sends a query's SequencedSecurityBookQuote.
     * @param book_quote The query's SequencedSecurityBookQuote.
     */
    (BookQuoteMessage, "Nexus.MarketDataService.BookQuoteMessage",
      SequencedSecurityBookQuote, book_quote),

    /**
     * Sends a query's SequencedSecurityTimeAndSale.
     * @param time_and_sale The query's SequencedSecurityTimeAndSale.
     */
    (TimeAndSaleMessage,  "Nexus.MarketDataService.TimeAndSaleMessage",
      SequencedSecurityTimeAndSale, time_and_sale),

    /**
     * Terminates a previous OrderImbalance query.
     * @param market The market that was queried.
     * @param id The id of query to end.
     */
    (EndOrderImbalanceQueryMessage,
      "Nexus.MarketDataService.EndOrderImbalanceQueryMessage", MarketCode,
      market, int, id),

    /**
     * Terminates a previous BboQuote query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndBboQuoteQueryMessage, "Nexus.MarketDataService.EndBboQuoteQueryMessage",
      Security, security, int, id),

    /**
     * Terminates a previous BookQuote query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndBookQuoteQueryMessage,
      "Nexus.MarketDataService.EndBookQuoteQueryMessage", Security, security,
      int, id),

    /**
     * Terminates a previous MarketQuote query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndMarketQuoteQueryMessage,
      "Nexus.MarketDataService.EndMarketQuoteQueryMessage", Security, security,
      int, id),

    /**
     * Terminates a previous TimeAndSale query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndTimeAndSaleQueryMessage,
      "Nexus.MarketDataService.EndTimeAndSaleQueryMessage", Security, security,
      int, id));

  /**
   * Returns the type of Service Message used to publish an update to a market
   * data query.
   * @param <T> The type of market data to publish.
   */
  template<typename T>
  struct MarketDataMessageType {};

  template<typename T>
  using GetMarketDataMessageType = typename MarketDataMessageType<T>::type;

  template<>
  struct MarketDataMessageType<OrderImbalance> {
    using type = OrderImbalanceMessage;
  };

  template<>
  struct MarketDataMessageType<BboQuote> {
    using type = BboQuoteMessage;
  };

  template<>
  struct MarketDataMessageType<MarketQuote> {
    using type = MarketQuoteMessage;
  };

  template<>
  struct MarketDataMessageType<BookQuote> {
    using type = BookQuoteMessage;
  };

  template<>
  struct MarketDataMessageType<TimeAndSale> {
    using type = TimeAndSaleMessage;
  };
}

#endif
