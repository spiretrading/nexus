#ifndef NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#include <Beam/Collections/AnyIterator.hpp>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Queries/LocalDataStore.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /** Stores historical market data in memory. */
  class LocalHistoricalDataStore {
    public:

      /** Constructs a LocalHistoricalDataStore. */
      LocalHistoricalDataStore() = default;

      /** Returns all OrderImbalances stored. */
      std::vector<SequencedMarketOrderImbalance> LoadOrderImbalances();

      /** Returns all the BboQuotes stored. */
      std::vector<SequencedSecurityBboQuote> LoadBboQuotes();

      /** Returns all the MarketQuotes stored. */
      std::vector<SequencedSecurityMarketQuote> LoadMarketQuotes();

      /** Returns all the BookQuotes stored. */
      std::vector<SequencedSecurityBookQuote> LoadBookQuotes();

      /** Returns all the TimeAndSales stored. */
      std::vector<SequencedSecurityTimeAndSale> LoadTimeAndSales();

      std::vector<SecurityInfo> LoadSecurityInfo(
        const SecurityInfoQuery& query);

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const SecurityMarketDataQuery& query);

      void Store(const SecurityInfo& info);

      void Store(const SequencedMarketOrderImbalance& orderImbalance);

      void Store(const std::vector<SequencedMarketOrderImbalance>&
        orderImbalances);

      void Store(const SequencedSecurityBboQuote& bboQuote);

      void Store(const std::vector<SequencedSecurityBboQuote>& bboQuotes);

      void Store(const SequencedSecurityMarketQuote& marketQuote);

      void Store(const std::vector<SequencedSecurityMarketQuote>& marketQuotes);

      void Store(const SequencedSecurityBookQuote& bookQuote);

      void Store(const std::vector<SequencedSecurityBookQuote>& bookQuotes);

      void Store(const SequencedSecurityTimeAndSale& timeAndSale);

      void Store(const std::vector<SequencedSecurityTimeAndSale>& timeAndSales);

      void Close();

    private:
      template<typename T, typename Query>
      using DataStore = Beam::Queries::LocalDataStore<Query, T,
        Queries::EvaluatorTranslator>;
      Beam::SynchronizedVector<SecurityInfo> m_securityInfo;
      DataStore<OrderImbalance, MarketWideDataQuery> m_orderImbalanceDataStore;
      DataStore<BboQuote, SecurityMarketDataQuery> m_bboQuoteDataStore;
      DataStore<MarketQuote, SecurityMarketDataQuery> m_marketQuoteDataStore;
      DataStore<BookQuote, SecurityMarketDataQuery> m_bookQuoteDataStore;
      DataStore<TimeAndSale, SecurityMarketDataQuery> m_timeAndSaleDataStore;

      LocalHistoricalDataStore(const LocalHistoricalDataStore&) = delete;
      LocalHistoricalDataStore& operator =(
        const LocalHistoricalDataStore&) = delete;
  };

  inline std::vector<SequencedMarketOrderImbalance> LocalHistoricalDataStore::
      LoadOrderImbalances() {
    return m_orderImbalanceDataStore.LoadAll();
  }

  inline std::vector<SequencedSecurityBboQuote> LocalHistoricalDataStore::
      LoadBboQuotes() {
    return m_bboQuoteDataStore.LoadAll();
  }

  inline std::vector<SequencedSecurityMarketQuote> LocalHistoricalDataStore::
      LoadMarketQuotes() {
    return m_marketQuoteDataStore.LoadAll();
  }

  inline std::vector<SequencedSecurityBookQuote> LocalHistoricalDataStore::
      LoadBookQuotes() {
    return m_bookQuoteDataStore.LoadAll();
  }

  inline std::vector<SequencedSecurityTimeAndSale> LocalHistoricalDataStore::
      LoadTimeAndSales() {
    return m_timeAndSaleDataStore.LoadAll();
  }

  inline std::vector<SecurityInfo> LocalHistoricalDataStore::LoadSecurityInfo(
      const SecurityInfoQuery& query) {
    auto evaluator =
      Beam::Queries::Translate<Nexus::Queries::EvaluatorTranslator>(
        query.GetFilter());
    return m_securityInfo.With([&] (auto& securityInfo) {
      auto matches = std::vector<SecurityInfo>();
      auto [begin, end] = [&] {
        if(query.GetSnapshotLimit().GetType() ==
            Beam::Queries::SnapshotLimit::Type::HEAD) {
          return std::tuple(Beam::AnyIterator(securityInfo.begin()),
            Beam::AnyIterator(securityInfo.end()));
        }
        return std::tuple(Beam::AnyIterator(securityInfo.rbegin()),
          Beam::AnyIterator(securityInfo.rend()));
      }();
      if(auto anchor = query.GetAnchor()) {
        while(begin != end && begin->m_security != *anchor) {
          ++begin;
        }
        if(begin != end) {
          ++begin;
        }
      }
      while(begin != end && static_cast<int>(matches.size()) <
          query.GetSnapshotLimit().GetSize()) {
        auto& info = *begin;
        if(info.m_security <= query.GetIndex() &&
            Beam::Queries::TestFilter(*evaluator, info)) {
          matches.push_back(info);
        }
        ++begin;
      }
      if(query.GetSnapshotLimit().GetType() ==
          Beam::Queries::SnapshotLimit::Type::TAIL) {
        std::reverse(matches.begin(), matches.end());
      }
      return matches;
    });
  }

  inline std::vector<SequencedOrderImbalance> LocalHistoricalDataStore::
      LoadOrderImbalances(const MarketWideDataQuery& query) {
    return m_orderImbalanceDataStore.Load(query);
  }

  inline std::vector<SequencedBboQuote> LocalHistoricalDataStore::LoadBboQuotes(
      const SecurityMarketDataQuery& query) {
    return m_bboQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedBookQuote> LocalHistoricalDataStore::
      LoadBookQuotes(const SecurityMarketDataQuery& query) {
    return m_bookQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedMarketQuote> LocalHistoricalDataStore::
      LoadMarketQuotes(const SecurityMarketDataQuery& query) {
    return m_marketQuoteDataStore.Load(query);
  }

  inline std::vector<SequencedTimeAndSale> LocalHistoricalDataStore::
      LoadTimeAndSales(const SecurityMarketDataQuery& query) {
    return m_timeAndSaleDataStore.Load(query);
  }

  inline void LocalHistoricalDataStore::Store(const SecurityInfo& info) {
    m_securityInfo.With([&] (auto& securityInfo) {
      auto i = std::lower_bound(securityInfo.begin(), securityInfo.end(), info,
        [&] (const auto& left, const auto& right) {
          return left.m_security < right.m_security;
        });
      if(i == securityInfo.end() || i->m_security != info.m_security) {
        securityInfo.insert(i, info);
      } else {
        *i = info;
      }
    });
  }

  inline void LocalHistoricalDataStore::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_orderImbalanceDataStore.Store(orderImbalance);
  }

  inline void LocalHistoricalDataStore::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_orderImbalanceDataStore.Store(orderImbalances);
  }

  inline void LocalHistoricalDataStore::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_bboQuoteDataStore.Store(bboQuote);
  }

  inline void LocalHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_bboQuoteDataStore.Store(bboQuotes);
  }

  inline void LocalHistoricalDataStore::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_marketQuoteDataStore.Store(marketQuote);
  }

  inline void LocalHistoricalDataStore::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_marketQuoteDataStore.Store(marketQuotes);
  }

  inline void LocalHistoricalDataStore::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_bookQuoteDataStore.Store(bookQuote);
  }

  inline void LocalHistoricalDataStore::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_bookQuoteDataStore.Store(bookQuotes);
  }

  inline void LocalHistoricalDataStore::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_timeAndSaleDataStore.Store(timeAndSale);
  }

  inline void LocalHistoricalDataStore::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_timeAndSaleDataStore.Store(timeAndSales);
  }

  inline void LocalHistoricalDataStore::Close() {}
}

#endif
