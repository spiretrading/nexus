#ifndef NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#define NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/QueryReactor.hpp>
#include "Nexus/MarketDataService/MarketDataClient.hpp"

namespace Nexus {

  /**
   * Returns a reactor that queries BboQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename QueryReactor>
  auto make_bbo_quote_reactor(
      IsMarketDataClient auto& client, QueryReactor&& query) {
    return Beam::query_reactor<BboQuote>(
      [&] (const auto& query, const auto& queue) {
        client.query(query, queue);
      }, std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current BboQuote.
   * @param client The market data client to query.
   * @param ticker The ticker whose current BboQuotes are to be queried.
   */
  template<typename TickerReactor>
  auto make_current_bbo_quote_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_bbo_quote_reactor(
      client, Aspen::lift(&Beam::make_current_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }

  /**
   * Returns a reactor that queries for real time BboQuotes.
   * @param client The market data client to query.
   * @param ticker The ticker whose real time BboQuotes are to be queried.
   */
  template<typename TickerReactor>
  auto make_real_time_bbo_quote_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_bbo_quote_reactor(
      client, Aspen::lift(&Beam::make_real_time_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }

  /**
   * Returns a reactor that queries BookQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename QueryReactor>
  auto make_book_quote_reactor(
      IsMarketDataClient auto& client, QueryReactor&& query) {
    return Beam::query_reactor<BookQuote>(
      [&] (const auto& query, const auto& queue) {
        client.query(query, queue);
      }, std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current BookQuote.
   * @param client The market data client to query.
   * @param ticker The ticker whose current BookQuotes are to be queried.
   */
  template<typename TickerReactor>
  auto make_current_book_quote_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_book_quote_reactor(
      client, Aspen::lift(&Beam::make_current_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }

  /**
   * Returns a reactor that queries for real time BookQuotes.
   * @param client The market data client to query.
   * @param ticker The ticker whose real time BookQuotes are to be queried.
   */
  template<typename TickerReactor>
  auto make_real_time_book_quote_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_book_quote_reactor(
      client, Aspen::lift(&Beam::make_real_time_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }

  /**
   * Returns a reactor that queries TimeAndSales.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename QueryReactor>
  auto make_time_and_sales_reactor(
      IsMarketDataClient auto& client, QueryReactor&& query) {
    return Beam::query_reactor<TimeAndSale>(
      [&] (const auto& query, const auto& queue) {
        client.query(query, queue);
      }, std::forward<QueryReactor>(query));
  }

  /**
   * Returns a reactor that queries the current TimeAndSales.
   * @param client The market data client to query.
   * @param ticker The ticker whose current TimeAndSales are to be queried.
   */
  template<typename TickerReactor>
  auto make_current_time_and_sales_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_time_and_sales_reactor(
      client, Aspen::lift(&Beam::make_current_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }

  /**
   * Returns a reactor that queries for real time TimeAndSales.
   * @param client The market data client to query.
   * @param ticker The ticker whose real time TimeAndSales are to be queried.
   */
  template<typename TickerReactor>
  auto make_real_time_time_and_sales_reactor(
      IsMarketDataClient auto& client, TickerReactor&& ticker) {
    return make_time_and_sales_reactor(
      client, Aspen::lift(&Beam::make_real_time_query<Ticker>,
        std::forward<TickerReactor>(ticker)));
  }
}

#endif
