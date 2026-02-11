#ifndef NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#include <Beam/Collections/AnyIterator.hpp>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Queries/LocalDataStore.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus {

  /** Implements an HistoricalDataStore in memory. */
  class LocalHistoricalDataStore {
    public:

      /** Constructs a LocalHistoricalDataStore. */
      LocalHistoricalDataStore() = default;

      /** Returns all OrderImbalances stored. */
      std::vector<SequencedVenueOrderImbalance> load_order_imbalances();

      /** Returns all the BboQuotes stored. */
      std::vector<SequencedTickerBboQuote> load_bbo_quotes();

      /** Returns all the BookQuotes stored. */
      std::vector<SequencedTickerBookQuote> load_book_quotes();

      /** Returns all the TimeAndSales stored. */
      std::vector<SequencedTickerTimeAndSale> load_time_and_sales();

      std::vector<TickerInfo> load_ticker_info(const TickerInfoQuery& query);
      void store(const TickerInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBboQuote& quote);
      void store(const std::vector<SequencedTickerBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerBookQuote& quote);
      void store(const std::vector<SequencedTickerBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const TickerMarketDataQuery& query);
      void store(const SequencedTickerTimeAndSale& time_and_sale);
      void store(const std::vector<SequencedTickerTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename T, typename Query>
      using DataStore = Beam::LocalDataStore<Query, T, EvaluatorTranslator>;
      Beam::SynchronizedVector<TickerInfo> m_ticker_info;
      DataStore<OrderImbalance, VenueMarketDataQuery>
        m_order_imbalance_data_store;
      DataStore<BboQuote, TickerMarketDataQuery> m_bbo_quote_data_store;
      DataStore<BookQuote, TickerMarketDataQuery> m_book_quote_data_store;
      DataStore<TimeAndSale, TickerMarketDataQuery> m_time_and_sale_data_store;

      LocalHistoricalDataStore(const LocalHistoricalDataStore&) = delete;
      LocalHistoricalDataStore& operator =(
        const LocalHistoricalDataStore&) = delete;
  };

  inline std::vector<SequencedVenueOrderImbalance>
      LocalHistoricalDataStore::load_order_imbalances() {
    return m_order_imbalance_data_store.load_all();
  }

  inline std::vector<SequencedTickerBboQuote>
      LocalHistoricalDataStore::load_bbo_quotes() {
    return m_bbo_quote_data_store.load_all();
  }

  inline std::vector<SequencedTickerBookQuote>
      LocalHistoricalDataStore::load_book_quotes() {
    return m_book_quote_data_store.load_all();
  }

  inline std::vector<SequencedTickerTimeAndSale>
      LocalHistoricalDataStore::load_time_and_sales() {
    return m_time_and_sale_data_store.load_all();
  }

  inline std::vector<TickerInfo> LocalHistoricalDataStore::load_ticker_info(
      const TickerInfoQuery& query) {
    auto evaluator = Beam::translate<EvaluatorTranslator>(query.get_filter());
    return m_ticker_info.with([&] (auto& ticker_info) {
      auto matches = std::vector<TickerInfo>();
      auto [begin, end] = [&] {
        if(query.get_snapshot_limit().get_type() ==
            Beam::SnapshotLimit::Type::HEAD) {
          return std::tuple(Beam::AnyIterator(ticker_info.begin()),
            Beam::AnyIterator(ticker_info.end()));
        }
        return std::tuple(Beam::AnyIterator(ticker_info.rbegin()),
          Beam::AnyIterator(ticker_info.rend()));
      }();
      if(auto anchor = query.get_anchor()) {
        while(begin != end && begin->m_ticker != *anchor) {
          ++begin;
        }
        if(begin != end) {
          ++begin;
        }
      }
      while(begin != end && static_cast<int>(matches.size()) <
          query.get_snapshot_limit().get_size()) {
        auto& info = *begin;
        if(info.m_ticker <= query.get_index() &&
            Beam::test_filter(*evaluator, info)) {
          matches.push_back(info);
        }
        ++begin;
      }
      if(query.get_snapshot_limit().get_type() ==
          Beam::SnapshotLimit::Type::TAIL) {
        std::reverse(matches.begin(), matches.end());
      }
      return matches;
    });
  }

  inline void LocalHistoricalDataStore::store(const TickerInfo& info) {
    m_ticker_info.with([&] (auto& ticker_info) {
      auto i = std::lower_bound(ticker_info.begin(), ticker_info.end(), info,
        [&] (const auto& left, const auto& right) {
          return left.m_ticker < right.m_ticker;
        });
      if(i == ticker_info.end() || i->m_ticker != info.m_ticker) {
        ticker_info.insert(i, info);
      } else {
        *i = info;
      }
    });
  }

  inline std::vector<SequencedOrderImbalance> LocalHistoricalDataStore::
      load_order_imbalances(const VenueMarketDataQuery& query) {
    return m_order_imbalance_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_order_imbalance_data_store.store(imbalance);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_order_imbalance_data_store.store(imbalances);
  }

  inline std::vector<SequencedBboQuote>
      LocalHistoricalDataStore::load_bbo_quotes(
        const TickerMarketDataQuery& query) {
    return m_bbo_quote_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedTickerBboQuote& quote) {
    m_bbo_quote_data_store.store(quote);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedTickerBboQuote>& quotes) {
    m_bbo_quote_data_store.store(quotes);
  }

  inline std::vector<SequencedBookQuote>
      LocalHistoricalDataStore::load_book_quotes(
        const TickerMarketDataQuery& query) {
    return m_book_quote_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedTickerBookQuote& quote) {
    m_book_quote_data_store.store(quote);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedTickerBookQuote>& quotes) {
    m_book_quote_data_store.store(quotes);
  }

  inline std::vector<SequencedTimeAndSale>
      LocalHistoricalDataStore::load_time_and_sales(
        const TickerMarketDataQuery& query) {
    return m_time_and_sale_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedTickerTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.store(time_and_sale);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedTickerTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.store(time_and_sales);
  }

  inline void LocalHistoricalDataStore::close() {}
}

#endif
