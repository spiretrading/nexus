#ifndef NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#include <string>
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Definitions/TickerInfo.hpp"
#include "Nexus/MarketDataService/TickerMarketDataQuery.hpp"
#include "Nexus/MarketDataService/TickerSnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
  using OrderImbalanceQueryResult = Beam::QueryResult<SequencedOrderImbalance>;
  using BboQuoteQueryResult = Beam::QueryResult<SequencedBboQuote>;
  using BookQuoteQueryResult = Beam::QueryResult<SequencedBookQuote>;
  using TimeAndSaleQueryResult = Beam::QueryResult<SequencedTimeAndSale>;

  /** Standard name for the market data registry service. */
  inline const auto MARKET_DATA_REGISTRY_SERVICE_NAME =
    std::string("market_data_registry_service");

  /** Standard name for the market data relay service. */
  inline const auto MARKET_DATA_RELAY_SERVICE_NAME =
    std::string("market_data_relay_service");

  BEAM_DEFINE_SERVICES(market_data_registry_services,

    /**
     * Queries a Market's OrderImbalances.
     * @param query The query to run.
     * @return The list of OrderImbalances satisfying the query.
     */
    (QueryOrderImbalancesService,
      "Nexus.MarketDataService.QueryOrderImbalancesService",
      OrderImbalanceQueryResult, (VenueMarketDataQuery, query)),

    /**
     * Queries a Ticker's BboQuotes.
     * @param query The query to run.
     * @return The list of BboQuotes satisfying the query.
     */
    (QueryBboQuotesService, "Nexus.MarketDataService.QueryBboQuotesService",
      BboQuoteQueryResult, (TickerMarketDataQuery, query)),

    /**
     * Queries a Ticker's BookQuotes.
     * @param query The query to run.
     * @return The list of BookQuotes satisfying the query.
     */
    (QueryBookQuotesService, "Nexus.MarketDataService.QueryBookQuotesService",
      BookQuoteQueryResult, (TickerMarketDataQuery, query)),

    /**
     * Queries a Ticker's TimeAndSales.
     * @param query The query to run.
     * @return The list of TimeAndSales satisfying the query.
     */
    (QueryTimeAndSalesService,
      "Nexus.MarketDataService.QueryTimeAndSalesService",
      TimeAndSaleQueryResult, (TickerMarketDataQuery, query)),

    /**
     * Loads a Ticker's real-time snapshot.
     * @param ticker The Ticker whose snapshot is to be loaded.
     * @return The TickerSnapshot for the specified <i>ticker</i>.
     */
    (LoadTickerSnapshotService,
      "Nexus.MarketDataService.LoadTickerSnapshotService", TickerSnapshot,
      (Ticker, ticker)),

    /**
     * Loads the session candlestick for a specified Ticker.
     * @param ticker The Ticker whose session candlestick is to be loaded.
     * @return The PriceCandlestick for the specified <i>ticker</i>.
     */
    (LoadSessionCandlestickService,
      "Nexus.MarketDataService.LoadSessionCandlestickService",
      PriceCandlestick, (Ticker, ticker)),

    /**
     * Queries for all TickerInfo objects that are within a region.
     * @param ticker The Ticker whose TickerInfo is to be loaded.
     * @return The list of TickerInfo objects that match the <i>query</i>.
     */
    (QueryTickerInfoService, "Nexus.MarketDataService.QueryTickerInfoService",
      std::vector<TickerInfo>, (TickerInfoQuery, query)),

    /**
     * Loads all TickerInfo objects that match a prefix.
     * @param prefix The prefix to search for.
     * @return The list of TickerInfo objects that match the <i>prefix</i>.
     */
    (LoadTickerInfoFromPrefixService,
      "Nexus.MarketDataService.LoadTickerInfoFromPrefixService",
      std::vector<TickerInfo>, (std::string, prefix)));

  BEAM_DEFINE_MESSAGES(market_data_registry_messages,

    /**
     * Sends a query's SequencedVenueOrderImbalance.
     * @param order_imbalance The query's SequencedVenueOrderImbalance.
     */
    (OrderImbalanceMessage, "Nexus.MarketDataService.OrderImbalanceMessage",
      (SequencedVenueOrderImbalance, order_imbalance)),

    /**
     * Sends a query's SequencedTickerBboQuote.
     * @param bbo_quote The query's SequencedTickerBboQuote.
     */
    (BboQuoteMessage, "Nexus.MarketDataService.BboQuoteMessage",
      (SequencedTickerBboQuote, bbo_quote)),

    /**
     * Sends a query's SequencedTickerBookQuote.
     * @param book_quote The query's SequencedTickerBookQuote.
     */
    (BookQuoteMessage, "Nexus.MarketDataService.BookQuoteMessage",
      (SequencedTickerBookQuote, book_quote)),

    /**
     * Sends a query's SequencedTickerTimeAndSale.
     * @param time_and_sale The query's SequencedTickerTimeAndSale.
     */
    (TimeAndSaleMessage,  "Nexus.MarketDataService.TimeAndSaleMessage",
      (SequencedTickerTimeAndSale, time_and_sale)),

    /**
     * Terminates a previous OrderImbalance query.
     * @param venue The venue that was queried.
     * @param id The id of query to end.
     */
    (EndOrderImbalanceQueryMessage,
      "Nexus.MarketDataService.EndOrderImbalanceQueryMessage", (Venue, venue),
      (int, id)),

    /**
     * Terminates a previous BboQuote query.
     * @param ticker The Ticker that was queried.
     * @param id The id of query to end.
     */
    (EndBboQuoteQueryMessage, "Nexus.MarketDataService.EndBboQuoteQueryMessage",
      (Ticker, ticker), (int, id)),

    /**
     * Terminates a previous BookQuote query.
     * @param ticker The Ticker that was queried.
     * @param id The id of query to end.
     */
    (EndBookQuoteQueryMessage,
      "Nexus.MarketDataService.EndBookQuoteQueryMessage", (Ticker, ticker),
      (int, id)),

    /**
     * Terminates a previous TimeAndSale query.
     * @param ticker The Ticker that was queried.
     * @param id The id of query to end.
     */
    (EndTimeAndSaleQueryMessage,
      "Nexus.MarketDataService.EndTimeAndSaleQueryMessage",
      (Ticker, ticker), (int, id)));

  /**
   * Returns the type of Service Message used to publish an update to a market
   * data query.
   * @param <T> The type of market data to publish.
   */
  template<typename T>
  struct market_data_message_type {};

  template<typename T>
  using market_data_message_type_t = typename market_data_message_type<T>::type;

  template<>
  struct market_data_message_type<OrderImbalance> {
    using type = OrderImbalanceMessage;
  };

  template<>
  struct market_data_message_type<BboQuote> {
    using type = BboQuoteMessage;
  };

  template<>
  struct market_data_message_type<BookQuote> {
    using type = BookQuoteMessage;
  };

  template<>
  struct market_data_message_type<TimeAndSale> {
    using type = TimeAndSaleMessage;
  };
}

#endif
