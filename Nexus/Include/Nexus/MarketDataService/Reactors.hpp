#ifndef NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#define NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Reactors/QueryReactor.hpp>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /**
   * Returns a reactor that queries BboQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto make_bbo_quote_reactor(MarketDataClient&& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<BboQuote>(
      [client = Beam::CapturePtr<MarketDataClient>(client)] (
          const auto& query, const auto& queue) {
        client->query(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current BboQuote.
   * @param client The market data client to query.
   * @param security The security whose current BboQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_current_bbo_quote_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_bbo_quote_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeCurrentQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }

  /**
   * Returns a reactor that queries for real time BboQuotes.
   * @param client The market data client to query.
   * @param security The security whose real time BboQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_real_time_bbo_quote_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_bbo_quote_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeRealTimeQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }

  /**
   * Returns a reactor that queries BookQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto make_book_quote_reactor(
      MarketDataClient&& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<BookQuote>(
      [client = Beam::CapturePtr<MarketDataClient>(client)] (
          const auto& query, const auto& queue) {
        client->query(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current BookQuote.
   * @param client The market data client to query.
   * @param security The security whose current BookQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_current_book_quote_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_book_quote_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeCurrentQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }

  /**
   * Returns a reactor that queries for real time BookQuotes.
   * @param client The market data client to query.
   * @param security The security whose real time BookQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_real_time_book_quote_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_book_quote_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeRealTimeQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }

  /**
   * Returns a reactor that queries TimeAndSales.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto make_time_and_sales_reactor(
      MarketDataClient&& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<TimeAndSale>(
      [client = Beam::CapturePtr<MarketDataClient>(client)] (
          const auto& query, const auto& queue) {
        client->query(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current TimeAndSales.
   * @param client The market data client to query.
   * @param security The security whose current TimeAndSales are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_current_time_and_sales_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_time_and_sales_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeCurrentQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }

  /**
   * Returns a reactor that queries for real time TimeAndSales.
   * @param client The market data client to query.
   * @param security The security whose real time TimeAndSales are to be
   *        queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto make_real_time_time_and_sales_reactor(
      MarketDataClient&& client, SecurityReactor&& security) {
    return make_time_and_sales_reactor(std::forward<MarketDataClient>(client),
      Aspen::lift(&Beam::Queries::MakeRealTimeQuery<Security>,
        std::forward<SecurityReactor>(security)));
  }
}

#endif
