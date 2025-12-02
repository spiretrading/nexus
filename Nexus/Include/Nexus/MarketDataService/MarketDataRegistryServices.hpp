#ifndef NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#define NEXUS_MARKET_DATA_REGISTRY_SERVICES_HPP
#include <string>
#include <Beam/Queries/QueryResult.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Nexus/Definitions/SecurityTechnicals.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/MarketDataService/SecuritySnapshot.hpp"
#include "Nexus/MarketDataService/VenueMarketDataQuery.hpp"

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
     * Queries a Security's BboQuotes.
     * @param query The query to run.
     * @return The list of BboQuotes satisfying the query.
     */
    (QueryBboQuotesService, "Nexus.MarketDataService.QueryBboQuotesService",
      BboQuoteQueryResult, (SecurityMarketDataQuery, query)),

    /**
     * Queries a Security's BookQuotes.
     * @param query The query to run.
     * @return The list of BookQuotes satisfying the query.
     */
    (QueryBookQuotesService, "Nexus.MarketDataService.QueryBookQuotesService",
      BookQuoteQueryResult, (SecurityMarketDataQuery, query)),

    /**
     * Queries a Security's TimeAndSales.
     * @param query The query to run.
     * @return The list of TimeAndSales satisfying the query.
     */
    (QueryTimeAndSalesService,
      "Nexus.MarketDataService.QueryTimeAndSalesService",
      TimeAndSaleQueryResult, (SecurityMarketDataQuery, query)),

    /**
     * Loads a Security's real-time snapshot.
     * @param security The Security whose snapshot is to be loaded.
     * @return The SecuritySnapshot for the specified <i>security</i>.
     */
    (LoadSecuritySnapshotService,
      "Nexus.MarketDataService.LoadSecuritySnapshotService", SecuritySnapshot,
      (Security, security)),

    /**
     * Loads the SecurityTechnicals for a specified Security.
     * @param security The Security whose SecurityTechnicals is to be loaded.
     * @return The SecurityTechnicals for the specified <i>security</i>.
     */
    (LoadSecurityTechnicalsService,
      "Nexus.MarketDataService.LoadSecurityTechnicalsService",
      SecurityTechnicals, (Security, security)),

    /**
     * Queries for all SecurityInfo objects that are within a region.
     * @param security The Security whose SecurityInfo is to be loaded.
     * @return The list of SecurityInfo objects that match the <i>query</i>.
     */
    (QuerySecurityInfoService,
      "Nexus.MarketDataService.QuerySecurityInfoService",
      std::vector<SecurityInfo>, (SecurityInfoQuery, query)),

    /**
     * Loads all SecurityInfo objects that match a prefix.
     * @param prefix The prefix to search for.
     * @return The list of SecurityInfo objects that match the <i>prefix</i>.
     */
    (LoadSecurityInfoFromPrefixService,
      "Nexus.MarketDataService.LoadSecurityInfoFromPrefixService",
      std::vector<SecurityInfo>, (std::string, prefix)));

  BEAM_DEFINE_MESSAGES(market_data_registry_messages,

    /**
     * Sends a query's SequencedVenueOrderImbalance.
     * @param order_imbalance The query's SequencedVenueOrderImbalance.
     */
    (OrderImbalanceMessage, "Nexus.MarketDataService.OrderImbalanceMessage",
      (SequencedVenueOrderImbalance, order_imbalance)),

    /**
     * Sends a query's SequencedSecurityBboQuote.
     * @param bbo_quote The query's SequencedSecurityBboQuote.
     */
    (BboQuoteMessage, "Nexus.MarketDataService.BboQuoteMessage",
      (SequencedSecurityBboQuote, bbo_quote)),

    /**
     * Sends a query's SequencedSecurityBookQuote.
     * @param book_quote The query's SequencedSecurityBookQuote.
     */
    (BookQuoteMessage, "Nexus.MarketDataService.BookQuoteMessage",
      (SequencedSecurityBookQuote, book_quote)),

    /**
     * Sends a query's SequencedSecurityTimeAndSale.
     * @param time_and_sale The query's SequencedSecurityTimeAndSale.
     */
    (TimeAndSaleMessage,  "Nexus.MarketDataService.TimeAndSaleMessage",
      (SequencedSecurityTimeAndSale, time_and_sale)),

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
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndBboQuoteQueryMessage, "Nexus.MarketDataService.EndBboQuoteQueryMessage",
      (Security, security), (int, id)),

    /**
     * Terminates a previous BookQuote query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndBookQuoteQueryMessage,
      "Nexus.MarketDataService.EndBookQuoteQueryMessage", (Security, security),
      (int, id)),

    /**
     * Terminates a previous TimeAndSale query.
     * @param security The Security that was queried.
     * @param id The id of query to end.
     */
    (EndTimeAndSaleQueryMessage,
      "Nexus.MarketDataService.EndTimeAndSaleQueryMessage",
      (Security, security), (int, id)));

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
