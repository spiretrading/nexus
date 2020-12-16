#ifndef NEXUS_BACKTESTER_HISTORICAL_DATA_STORE_HPP
#define NEXUS_BACKTESTER_HISTORICAL_DATA_STORE_HPP
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/CutoffHistoricalDataStore.hpp"

namespace Nexus {

  /**
   * Historical data store used by the backtester.
   * @param <H> The underlying data store to wrap.
   */
  template<typename H>
  class BacktesterHistoricalDataStore {
    public:

      /** The type of underlying data store to wrap. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<H>;

      /**
       * Constructs a BacktesterHistoricalDataStore.
       * @param dataStore Initializes the data store to wrap.
       * @param cutoff The date/time to satisfied queries to.
       */
      template<typename D>
      BacktesterHistoricalDataStore(D&& dataStore,
        boost::posix_time::ptime cutoff);

      ~BacktesterHistoricalDataStore();

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
      CutoffHistoricalDataStore<H> m_dataStore;

      BacktesterHistoricalDataStore(
        const BacktesterHistoricalDataStore&) = delete;
      BacktesterHistoricalDataStore& operator =(
        const BacktesterHistoricalDataStore&) = delete;
  };

  template<typename H>
  template<typename D>
  BacktesterHistoricalDataStore<H>::BacktesterHistoricalDataStore(D&& dataStore,
    boost::posix_time::ptime cutoff)
    : m_dataStore(std::forward<D>(dataStore), cutoff) {}

  template<typename H>
  BacktesterHistoricalDataStore<H>::~BacktesterHistoricalDataStore() {
    Close();
  }

  template<typename H>
  std::vector<SecurityInfo> BacktesterHistoricalDataStore<H>::LoadSecurityInfo(
      const MarketDataService::SecurityInfoQuery& query) {
    return m_dataStore.LoadSecurityInfo(query);
  }

  template<typename H>
  std::vector<SequencedOrderImbalance>
      BacktesterHistoricalDataStore<H>::LoadOrderImbalances(
      const MarketDataService::MarketWideDataQuery& query) {
    return m_dataStore.LoadOrderImbalances(query);
  }

  template<typename H>
  std::vector<SequencedBboQuote>
      BacktesterHistoricalDataStore<H>::LoadBboQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return m_dataStore.LoadBboQuotes(query);
  }

  template<typename H>
  std::vector<SequencedBookQuote>
      BacktesterHistoricalDataStore<H>::LoadBookQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return m_dataStore.LoadBookQuotes(query);
  }

  template<typename H>
  std::vector<SequencedMarketQuote>
      BacktesterHistoricalDataStore<H>::LoadMarketQuotes(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return m_dataStore.LoadMarketQuotes(query);
  }

  template<typename H>
  std::vector<SequencedTimeAndSale>
      BacktesterHistoricalDataStore<H>::LoadTimeAndSales(
      const MarketDataService::SecurityMarketDataQuery& query) {
    return m_dataStore.LoadTimeAndSales(query);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(const SecurityInfo& info) {
    m_dataStore.Store(info);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const SequencedMarketOrderImbalance& orderImbalance) {
    m_dataStore.Store(orderImbalance);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const std::vector<SequencedMarketOrderImbalance>& orderImbalances) {
    m_dataStore.Store(orderImbalances);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const SequencedSecurityBboQuote& bboQuote) {
    m_dataStore.Store(bboQuote);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityBboQuote>& bboQuotes) {
    m_dataStore.Store(bboQuotes);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const SequencedSecurityMarketQuote& marketQuote) {
    m_dataStore.Store(marketQuote);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityMarketQuote>& marketQuotes) {
    m_dataStore.Store(marketQuotes);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const SequencedSecurityBookQuote& bookQuote) {
    m_dataStore.Store(bookQuote);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityBookQuote>& bookQuotes) {
    m_dataStore.Store(bookQuotes);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const SequencedSecurityTimeAndSale& timeAndSale) {
    m_dataStore.Store(timeAndSale);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Store(
      const std::vector<SequencedSecurityTimeAndSale>& timeAndSales) {
    m_dataStore.Store(timeAndSales);
  }

  template<typename H>
  void BacktesterHistoricalDataStore<H>::Close() {
    m_dataStore.Close();
  }
}

#endif
