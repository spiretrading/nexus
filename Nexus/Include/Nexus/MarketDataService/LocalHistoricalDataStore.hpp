#ifndef NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_LOCAL_HISTORICAL_DATA_STORE_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/Queries/LocalDataStore.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/HistoricalDataStore.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /** Stores historical market data in memory. */
  class LocalHistoricalDataStore : private boost::noncopyable {
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

      boost::optional<SecurityInfo> LoadSecurityInfo(const Security& security);

      std::vector<SecurityInfo> LoadAllSecurityInfo();

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

      void Open();

      void Close();

    private:
      template<typename T, typename Query>
      using DataStore = Beam::Queries::LocalDataStore<Query, T,
        Queries::EvaluatorTranslator>;
      Beam::SynchronizedUnorderedMap<Security, SecurityInfo> m_securityInfo;
      DataStore<OrderImbalance, MarketWideDataQuery> m_orderImbalanceDataStore;
      DataStore<BboQuote, SecurityMarketDataQuery> m_bboQuoteDataStore;
      DataStore<MarketQuote, SecurityMarketDataQuery> m_marketQuoteDataStore;
      DataStore<BookQuote, SecurityMarketDataQuery> m_bookQuoteDataStore;
      DataStore<TimeAndSale, SecurityMarketDataQuery> m_timeAndSaleDataStore;
  };

  inline boost::optional<SecurityInfo> LocalHistoricalDataStore::
      LoadSecurityInfo(const Security& security) {
    auto info = m_securityInfo.Find(security);
    if(info.is_initialized()) {
      return *info;
    }
    return boost::none;
  }

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

  inline std::vector<SecurityInfo> LocalHistoricalDataStore::
      LoadAllSecurityInfo() {
    auto result = std::vector<SecurityInfo>();
    m_securityInfo.With(
      [&] (const auto& securityInfo) {
        for(auto& entry : securityInfo) {
          result.push_back(entry.second);
        }
      });
    return result;
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
    m_securityInfo.Insert(info.m_security, info);
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

  inline void LocalHistoricalDataStore::Open() {}

  inline void LocalHistoricalDataStore::Close() {}
}

#endif
