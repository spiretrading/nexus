#ifndef BACKTESTER_CUTOFF_HISTORICAL_DATA_STORE_HPP
#define BACKTESTER_CUTOFF_HISTORICAL_DATA_STORE_HPP
#include <unordered_map>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/lock_types.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"

namespace Nexus {

  /**
   * Wraps a historical market data store and satisfies queries submitted to it
   * up to a certain date time.
   * @param <H> The underlying data store to wrap.
   */
  template<typename H>
  class CutoffHistoricalDataStore {
    public:

      /** The type of underlying data store to wrap. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<H>;

      /**
       * Constructs a CutoffHistoricalDataStore.
       * @param dataStore Initializes the data store to wrap.
       * @param cutoff The date/time to satisfied queries to.
       */
      template<typename D>
      CutoffHistoricalDataStore(D&& dataStore, boost::posix_time::ptime cutoff);

      ~CutoffHistoricalDataStore();

      std::vector<SecurityInfo> LoadSecurityInfo(
        const MarketDataService::SecurityInfoQuery& query);

      std::vector<SequencedOrderImbalance> LoadOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query);

      std::vector<SequencedBboQuote> LoadBboQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      std::vector<SequencedBookQuote> LoadBookQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      std::vector<SequencedMarketQuote> LoadMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query);

      std::vector<SequencedTimeAndSale> LoadTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query);

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
      mutable Beam::Threading::Mutex m_mutex;
      Beam::GetOptionalLocalPtr<H> m_dataStore;
      boost::posix_time::ptime m_cutoff;
      std::unordered_map<MarketCode, Beam::Queries::Sequence>
        m_orderImbalanceCutoffSequences;
      std::unordered_map<Security, Beam::Queries::Sequence>
        m_bboQuoteCutoffSequences;
      std::unordered_map<Security, Beam::Queries::Sequence>
        m_bookQuoteCutoffSequences;
      std::unordered_map<Security, Beam::Queries::Sequence>
        m_marketQuoteCutoffSequences;
      std::unordered_map<Security, Beam::Queries::Sequence>
        m_timeAndSalesCutoffSequences;
      Beam::IO::OpenState m_openState;

      CutoffHistoricalDataStore(const CutoffHistoricalDataStore&) = delete;
      CutoffHistoricalDataStore& operator =(
        const CutoffHistoricalDataStore&) = delete;
      template<typename Query, typename F>
      std::invoke_result_t<F, const Query&> Load(const Query& query,
        std::unordered_map<typename Query::Index, Beam::Queries::Sequence>&
        cutoffSequences, F&& loader);
  };

  template<typename H>
  template<typename D>
  CutoffHistoricalDataStore<H>::CutoffHistoricalDataStore(D&& dataStore,
    boost::posix_time::ptime cutoff)
    : m_dataStore(std::forward<D>(dataStore)),
      m_cutoff(cutoff) {}

  template<typename H>
  CutoffHistoricalDataStore<H>::~CutoffHistoricalDataStore() {
    Close();
  }

  template<typename H>
  std::vector<SecurityInfo> CutoffHistoricalDataStore<H>::LoadSecurityInfo(
      const MarketDataService::SecurityInfoQuery& query) {
    return m_dataStore->LoadSecurityInfo(query);
  }

  template<typename H>
  std::vector<SequencedOrderImbalance>
      CutoffHistoricalDataStore<H>::LoadOrderImbalances(
        const MarketDataService::MarketWideDataQuery& query) {
    return Load(query, m_orderImbalanceCutoffSequences, [&] (auto& query) {
      return m_dataStore->LoadOrderImbalances(query);
    });
  }

  template<typename H>
  std::vector<SequencedBboQuote> CutoffHistoricalDataStore<H>::LoadBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return Load(query, m_bboQuoteCutoffSequences, [&] (auto& query) {
      return m_dataStore->LoadBboQuotes(query);
    });
  }

  template<typename H>
  std::vector<SequencedBookQuote> CutoffHistoricalDataStore<H>::LoadBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return Load(query, m_bookQuoteCutoffSequences, [&] (auto& query) {
      return m_dataStore->LoadBookQuotes(query);
    });
  }

  template<typename H>
  std::vector<SequencedMarketQuote>
      CutoffHistoricalDataStore<H>::LoadMarketQuotes(
        const MarketDataService::SecurityMarketDataQuery& query) {
    return Load(query, m_marketQuoteCutoffSequences, [&] (auto& query) {
      return m_dataStore->LoadMarketQuotes(query);
    });
  }

  template<typename H>
  std::vector<SequencedTimeAndSale>
      CutoffHistoricalDataStore<H>::LoadTimeAndSales(
        const MarketDataService::SecurityMarketDataQuery& query) {
    return Load(query, m_timeAndSalesCutoffSequences, [&] (auto& query) {
      return m_dataStore->LoadTimeAndSales(query);
    });
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(const SecurityInfo& info) {
    m_dataStore->Store(info);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore->Store(orderImbalance);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore->Store(orderImbalances);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_dataStore->Store(bboQuote);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_dataStore->Store(bboQuotes);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_dataStore->Store(marketQuote);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_dataStore->Store(marketQuotes);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_dataStore->Store(bookQuote);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_dataStore->Store(bookQuotes);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_dataStore->Store(timeAndSale);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_dataStore->Store(timeAndSales);
  }

  template<typename H>
  void CutoffHistoricalDataStore<H>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename H>
  template<typename Query, typename F>
  std::invoke_result_t<F, const Query&> CutoffHistoricalDataStore<H>::Load(
      const Query& query,
      std::unordered_map<typename Query::Index, Beam::Queries::Sequence>&
        cutoffSequences, F&& loader) {
    if(auto startTimestamp = boost::get<boost::posix_time::ptime>(
        &query.GetRange().GetStart())) {
      if(*startTimestamp >= m_cutoff) {
        return {};
      }
    }
    auto cutoffSequence = [&] {
      auto lock = boost::lock_guard(m_mutex);
      auto cutoff = cutoffSequences.find(query.GetIndex());
      auto rangeEnd = Beam::Queries::Range::Point(m_cutoff);
      while(cutoff == cutoffSequences.end()) {
        auto cutoffQuery = Query();
        cutoffQuery.SetIndex(query.GetIndex());
        cutoffQuery.SetRange(Beam::Queries::Range(
          Beam::Queries::Sequence::First(), rangeEnd));
        cutoffQuery.SetSnapshotLimit(
          Beam::Queries::SnapshotLimit::FromTail(100));
        auto sequences = loader(cutoffQuery);
        if(sequences.empty()) {
          cutoff = cutoffSequences.insert(std::make_pair(query.GetIndex(),
            Beam::Queries::Sequence::First())).first;
        } else {
          for(auto i = sequences.rbegin(); i != sequences.rend(); ++i) {
            if((*i)->m_timestamp < m_cutoff) {
              cutoff = cutoffSequences.insert(
                std::make_pair(query.GetIndex(), i->GetSequence())).first;
              break;
            }
          }
          rangeEnd = Beam::Queries::Decrement(sequences.front().GetSequence());
          if(rangeEnd == Beam::Queries::Sequence::First()) {
            cutoff = cutoffSequences.insert(std::make_pair(query.GetIndex(),
              Beam::Queries::Sequence::First())).first;
          }
        }
      }
      return cutoff->second;
    }();
    if(auto startSequence = boost::get<Beam::Queries::Sequence>(
        &query.GetRange().GetStart())) {
      if(*startSequence > cutoffSequence) {
        return {};
      }
    }
    auto cutoffQuery = query;
    cutoffQuery.SetRange(Beam::Queries::Range(query.GetRange().GetStart(),
      cutoffSequence));
    return std::forward<F>(loader)(cutoffQuery);
  }
}

#endif
