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
      std::vector<SequencedSecurityBboQuote> load_bbo_quotes();

      /** Returns all the BookQuotes stored. */
      std::vector<SequencedSecurityBookQuote> load_book_quotes();

      /** Returns all the TimeAndSales stored. */
      std::vector<SequencedSecurityTimeAndSale> load_time_and_sales();

      std::vector<SecurityInfo> load_security_info(
        const SecurityInfoQuery& query);
      void store(const SecurityInfo& info);
      std::vector<SequencedOrderImbalance> load_order_imbalances(
        const VenueMarketDataQuery& query);
      void store(const SequencedVenueOrderImbalance& imbalance);
      void store(const std::vector<SequencedVenueOrderImbalance>& imbalances);
      std::vector<SequencedBboQuote> load_bbo_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBboQuote& quote);
      void store(const std::vector<SequencedSecurityBboQuote>& quotes);
      std::vector<SequencedBookQuote> load_book_quotes(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityBookQuote& quote);
      void store(const std::vector<SequencedSecurityBookQuote>& quotes);
      std::vector<SequencedTimeAndSale> load_time_and_sales(
        const SecurityMarketDataQuery& query);
      void store(const SequencedSecurityTimeAndSale& time_and_sale);
      void store(
        const std::vector<SequencedSecurityTimeAndSale>& time_and_sales);
      void close();

    private:
      template<typename T, typename Query>
      using DataStore = Beam::LocalDataStore<Query, T, EvaluatorTranslator>;
      Beam::SynchronizedVector<SecurityInfo> m_security_info;
      DataStore<OrderImbalance, VenueMarketDataQuery>
        m_order_imbalance_data_store;
      DataStore<BboQuote, SecurityMarketDataQuery> m_bbo_quote_data_store;
      DataStore<BookQuote, SecurityMarketDataQuery> m_book_quote_data_store;
      DataStore<TimeAndSale, SecurityMarketDataQuery>
        m_time_and_sale_data_store;

      LocalHistoricalDataStore(const LocalHistoricalDataStore&) = delete;
      LocalHistoricalDataStore& operator =(
        const LocalHistoricalDataStore&) = delete;
  };

  inline std::vector<SequencedVenueOrderImbalance>
      LocalHistoricalDataStore::load_order_imbalances() {
    return m_order_imbalance_data_store.load_all();
  }

  inline std::vector<SequencedSecurityBboQuote>
      LocalHistoricalDataStore::load_bbo_quotes() {
    return m_bbo_quote_data_store.load_all();
  }

  inline std::vector<SequencedSecurityBookQuote>
      LocalHistoricalDataStore::load_book_quotes() {
    return m_book_quote_data_store.load_all();
  }

  inline std::vector<SequencedSecurityTimeAndSale>
      LocalHistoricalDataStore::load_time_and_sales() {
    return m_time_and_sale_data_store.load_all();
  }

  inline std::vector<SecurityInfo> LocalHistoricalDataStore::load_security_info(
      const SecurityInfoQuery& query) {
    auto evaluator = Beam::translate<EvaluatorTranslator>(query.get_filter());
    return m_security_info.with([&] (auto& security_info) {
      auto matches = std::vector<SecurityInfo>();
      auto [begin, end] = [&] {
        if(query.get_snapshot_limit().get_type() ==
            Beam::SnapshotLimit::Type::HEAD) {
          return std::tuple(Beam::AnyIterator(security_info.begin()),
            Beam::AnyIterator(security_info.end()));
        }
        return std::tuple(Beam::AnyIterator(security_info.rbegin()),
          Beam::AnyIterator(security_info.rend()));
      }();
      if(auto anchor = query.get_anchor()) {
        while(begin != end && begin->m_security != *anchor) {
          ++begin;
        }
        if(begin != end) {
          ++begin;
        }
      }
      while(begin != end && static_cast<int>(matches.size()) <
          query.get_snapshot_limit().get_size()) {
        auto& info = *begin;
        if(info.m_security <= query.get_index() &&
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

  inline void LocalHistoricalDataStore::store(const SecurityInfo& info) {
    m_security_info.with([&] (auto& security_info) {
      auto i = std::lower_bound(
        security_info.begin(), security_info.end(), info,
        [&] (const auto& left, const auto& right) {
          return left.m_security < right.m_security;
        });
      if(i == security_info.end() || i->m_security != info.m_security) {
        security_info.insert(i, info);
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
        const SecurityMarketDataQuery& query) {
    return m_bbo_quote_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedSecurityBboQuote& quote) {
    m_bbo_quote_data_store.store(quote);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_bbo_quote_data_store.store(quotes);
  }

  inline std::vector<SequencedBookQuote>
      LocalHistoricalDataStore::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return m_book_quote_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedSecurityBookQuote& quote) {
    m_book_quote_data_store.store(quote);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_book_quote_data_store.store(quotes);
  }

  inline std::vector<SequencedTimeAndSale>
      LocalHistoricalDataStore::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_time_and_sale_data_store.load(query);
  }

  inline void LocalHistoricalDataStore::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.store(time_and_sale);
  }

  inline void LocalHistoricalDataStore::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.store(time_and_sales);
  }

  inline void LocalHistoricalDataStore::close() {}
}

#endif
