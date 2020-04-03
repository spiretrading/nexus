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

    /*! \interface Nexus::MarketDataService::QueryOrderImbalancesService
        \brief Queries a Market's OrderImbalances.
        \param query <code>MarketWideDataQuery</code> The query to run.
        \return <code>OrderImbalanceQueryResult</code> The list of
                OrderImbalances satisfying the query.
    */
    //! \cond
    (QueryOrderImbalancesService,
      "Nexus.MarketDataService.QueryOrderImbalancesService",
      OrderImbalanceQueryResult, MarketWideDataQuery, query),
    //! \endcond

    /*! \interface Nexus::MarketDataService::QueryBboQuotesService
        \brief Queries a Security's BboQuotes.
        \param query <code>SecurityMarketDataQuery</code> The query to run.
        \return <code>BboQuoteQueryResult</code> The list of BboQuotes
                satisfying the query.
    */
    //! \cond
    (QueryBboQuotesService, "Nexus.MarketDataService.QueryBboQuotesService",
      BboQuoteQueryResult, SecurityMarketDataQuery, query),
    //! \endcond

    /*! \interface Nexus::MarketDataService::QueryBookQuotesService
        \brief Queries a Security's BookQuotes.
        \param query <code>SecurityMarketDataQuery</code> The query to run.
        \return <code>BookQuoteQueryResult</code> The list of BookQuotes
                satisfying the query.
    */
    //! \cond
    (QueryBookQuotesService, "Nexus.MarketDataService.QueryBookQuotesService",
      BookQuoteQueryResult, SecurityMarketDataQuery, query),
    //! \endcond

    /*! \interface Nexus::MarketDataService::QueryMarketQuotesService
        \brief Queries a Security's MarketQuotes.
        \param query <code>SecurityMarketDataQuery</code> The query to run.
        \return <code>MarketQuoteQueryResult</code> The list of MarketQuotes
                satisfying the query.
    */
    //! \cond
    (QueryMarketQuotesService,
      "Nexus.MarketDataService.QueryMarketQuotesService",
      MarketQuoteQueryResult, SecurityMarketDataQuery, query),
    //! \endcond

    /*! \interface Nexus::MarketDataService::QueryTimeAndSalesService
        \brief Queries a Security's TimeAndSales.
        \param query <code>SecurityMarketDataQuery</code> The query to run.
        \return <code>TimeAndSaleQueryResult</code> The list of TimeAndSales
                satisfying the query.
    */
    //! \cond
    (QueryTimeAndSalesService,
      "Nexus.MarketDataService.QueryTimeAndSalesService",
      TimeAndSaleQueryResult, SecurityMarketDataQuery, query),
    //! \endcond

    /*! \interface Nexus::MarketDataService::LoadSecuritySnapshotService
        \brief Loads a Security's real-time snapshot.
        \param security <code>Security</code> The Security whose snapshot is to
               be loaded.
        \return <code>SecuritySnapshot</code> The SecuritySnapshot for the
                specified <i>security</i>.
    */
    //! \cond
    (LoadSecuritySnapshotService,
      "Nexus.MarketDataService.LoadSecuritySnapshotService", SecuritySnapshot,
      Security, security),
    //! \endcond

    /*! \interface Nexus::MarketDataService::LoadSecurityTechnicalsService
        \brief Loads the SecurityTechnicals for a specified Security.
        \param security <code>Security</code> The Security whose
               SecurityTechnicals is to be loaded.
        \return <code>SecurityTechnicals</code> The SecurityTechnicals for the
                specified <i>security</i>.
    */
    //! \cond
    (LoadSecurityTechnicalsService,
      "Nexus.MarketDataService.LoadSecurityTechnicalsService",
      SecurityTechnicals, Security, security),
    //! \endcond

    /*! \interface Nexus::MarketDataService::LoadSecurityInfoService
        \brief Loads the SecurityInfo for a specified Security.
        \param security <code>Security</code> The Security whose SecurityInfo is
               to be loaded.
        \return <code>SecurityInfo</code> The SecurityInfo for the specified
                <i>security</i>.
    */
    //! \cond
    (LoadSecurityInfoService, "Nexus.MarketDataService.LoadSecurityInfoService",
      boost::optional<SecurityInfo>, Security, security),

    /*! \interface Nexus::MarketDataService::LoadSecurityInfoFromPrefixService
        \brief Loads all SecurityInfo objects that match a prefix.
        \param prefix <code>std::string</code> The prefix to search for.
        \return <code>std::vector\<SecurityInfo\></code> The list of
                SecurityInfo objects that match the <i>prefix</i>.
    */
    //! \cond
    (LoadSecurityInfoFromPrefixService,
      "Nexus.MarketDataService.LoadSecurityInfoFromPrefixService",
      std::vector<SecurityInfo>, std::string, prefix));
    //! \endcond

  BEAM_DEFINE_MESSAGES(MarketDataRegistryMessages,

    /*! \interface Nexus::MarketDataService::OrderImbalanceMessage
        \brief Sends a query's SequencedMarketOrderImbalance.
        \param order_imbalance <code>SequencedMarketOrderImbalance</code>
               The query's SequencedMarketOrderImbalance.
    */
    //! \cond
    (OrderImbalanceMessage, "Nexus.MarketDataService.OrderImbalanceMessage",
      SequencedMarketOrderImbalance, order_imbalance),
    //! \endcond

    /*! \interface Nexus::MarketDataService::BboQuoteMessage
        \brief Sends a query's SequencedSecurityBboQuote.
        \param bbo_quote <code>SequencedSecurityBboQuote</code>
               The query's SequencedSecurityBboQuote.
    */
    //! \cond
    (BboQuoteMessage, "Nexus.MarketDataService.BboQuoteMessage",
      SequencedSecurityBboQuote, bbo_quote),
    //! \endcond

    /*! \interface Nexus::MarketDataService::MarketQuoteMessage
        \brief Sends a query's SequencedSecurityMarketQuote.
        \param market_quote <code>SequencedSecurityMarketQuote</code>
               The query's SequencedSecurityMarketQuote.
    */
    //! \cond
    (MarketQuoteMessage, "Nexus.MarketDataService.MarketQuoteMessage",
      SequencedSecurityMarketQuote, market_quote),
    //! \endcond

    /*! \interface Nexus::MarketDataService::BookQuoteMessage
        \brief Sends a query's SequencedSecurityBookQuote.
        \param book_quote <code>SequencedSecurityBookQuote</code>
               The query's SequencedSecurityBookQuote.
    */
    //! \cond
    (BookQuoteMessage, "Nexus.MarketDataService.BookQuoteMessage",
      SequencedSecurityBookQuote, book_quote),
    //! \endcond

    /*! \interface Nexus::MarketDataService::TimeAndSaleMessage
        \brief Sends a query's SequencedSecurityTimeAndSale.
        \param time_and_sale <code>SequencedSecurityTimeAndSale</code>
               The query's SequencedSecurityTimeAndSale.
    */
    //! \cond
    (TimeAndSaleMessage,  "Nexus.MarketDataService.TimeAndSaleMessage",
      SequencedSecurityTimeAndSale, time_and_sale),
    //! \endcond

    /*! \interface Nexus::MarketDataService::EndOrderImbalanceQueryMessage
        \brief Terminates a previous OrderImbalance query.
        \param market <code>MarketCode</code> The market that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndOrderImbalanceQueryMessage,
      "Nexus.MarketDataService.EndOrderImbalanceQueryMessage", MarketCode,
      market, int, id),
    //! \endcond

    /*! \interface Nexus::MarketDataService::EndBboQuoteQueryMessage
        \brief Terminates a previous BboQuote query.
        \param security <code>Security</code> The Security that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndBboQuoteQueryMessage, "Nexus.MarketDataService.EndBboQuoteQueryMessage",
      Security, security, int, id),
    //! \endcond

    /*! \interface Nexus::MarketDataService::EndBookQuoteQueryMessage
        \brief Terminates a previous BookQuote query.
        \param security <code>Security</code> The Security that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndBookQuoteQueryMessage,
      "Nexus.MarketDataService.EndBookQuoteQueryMessage", Security, security,
      int, id),
    //! \endcond

    /*! \interface Nexus::MarketDataService::EndMarketQuoteQueryMessage
        \brief Terminates a previous MarketQuote query.
        \param security <code>Security</code> The Security that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndMarketQuoteQueryMessage,
      "Nexus.MarketDataService.EndMarketQuoteQueryMessage", Security, security,
      int, id),
    //! \endcond

    /*! \interface Nexus::MarketDataService::EndTimeAndSaleQueryMessage
        \brief Terminates a previous TimeAndSale query.
        \param security <code>Security</code> The Security that was queried.
        \param id <code>int</code> The id of query to end.
    */
    //! \cond
    (EndTimeAndSaleQueryMessage,
      "Nexus.MarketDataService.EndTimeAndSaleQueryMessage", Security, security,
      int, id));
    //! \endcond

  /*! \struct MarketDataMessageType
      \brief Returns the type of Service Message used to publish an update to a
             market data query.
      \tparam T The type of market data to publish.
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
