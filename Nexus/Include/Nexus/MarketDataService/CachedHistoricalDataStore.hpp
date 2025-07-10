#ifndef NEXUS_MARKET_DATA_CACHED_HISTORICAL_DATA_STORE_HPP
#define NEXUS_MARKET_DATA_CACHED_HISTORICAL_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/CachedDataStore.hpp>
#include "Nexus/MarketDataService/HistoricalDataStoreQueryWrapper.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"

namespace Nexus::MarketDataService {

  /**
   * Caches historical market data.
   * @param <D> The underlying data store to cache.
   */
  template<typename D>
  class CachedHistoricalDataStore {
    public:

      /** The type of HistoricalDataStore to cache. */
      using HistoricalDataStore = Beam::GetTryDereferenceType<D>;

      /**
       * Constructs a CachedHistoricalDataStore.
       * @param dataStore Initializes the data store to commit data to.
       * @param blockSize The size of a single cache block.
       */
      template<typename DF>
      CachedHistoricalDataStore(DF&& dataStore, int blockSize);

      ~CachedHistoricalDataStore();
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
      template<typename T>
      using DataStore = Beam::Queries::CachedDataStore<
        HistoricalDataStoreQueryWrapper<T, HistoricalDataStore*>,
        Queries::EvaluatorTranslator>;
      Beam::GetOptionalLocalPtr<D> m_data_store;
      DataStore<OrderImbalance> m_order_imbalance_data_store;
      DataStore<BboQuote> m_bbo_quote_data_store;
      DataStore<BookQuote> m_book_quote_data_store;
      DataStore<TimeAndSale> m_time_and_sale_data_store;
      Beam::IO::OpenState m_open_state;

      CachedHistoricalDataStore(const CachedHistoricalDataStore&) = delete;
      CachedHistoricalDataStore& operator =(
        const CachedHistoricalDataStore&) = delete;
  };

  template<typename D>
  template<typename DF>
  CachedHistoricalDataStore<D>::CachedHistoricalDataStore(
    DF&& data_store, int block_size)
    : m_data_store(std::forward<DF>(data_store)),
      m_order_imbalance_data_store(&*m_data_store, block_size),
      m_bbo_quote_data_store(&*m_data_store, block_size),
      m_book_quote_data_store(&*m_data_store, block_size),
      m_time_and_sale_data_store(&*m_data_store, block_size) {}

  template<typename D>
  CachedHistoricalDataStore<D>::~CachedHistoricalDataStore() {
    close();
  }

  template<typename D>
  std::vector<SecurityInfo> CachedHistoricalDataStore<D>::load_security_info(
      const SecurityInfoQuery& query) {
    return m_data_store->load_security_info(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(const SecurityInfo& info) {
    m_data_store->store(info);
  }

  template<typename D>
  std::vector<SequencedOrderImbalance>
      CachedHistoricalDataStore<D>::load_order_imbalances(
        const VenueMarketDataQuery& query) {
    return m_order_imbalance_data_store.Load(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const SequencedVenueOrderImbalance& imbalance) {
    m_order_imbalance_data_store.Store(imbalance);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const std::vector<SequencedVenueOrderImbalance>& imbalances) {
    m_order_imbalance_data_store.Store(imbalances);
  }

  template<typename D>
  std::vector<SequencedBboQuote>
      CachedHistoricalDataStore<D>::load_bbo_quotes(
        const SecurityMarketDataQuery& query) {
    return m_bbo_quote_data_store.Load(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const SequencedSecurityBboQuote& quote) {
    m_bbo_quote_data_store.Store(quote);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBboQuote>& quotes) {
    m_bbo_quote_data_store.Store(quotes);
  }

  template<typename D>
  std::vector<SequencedBookQuote>
      CachedHistoricalDataStore<D>::load_book_quotes(
        const SecurityMarketDataQuery& query) {
    return m_book_quote_data_store.Load(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const SequencedSecurityBookQuote& quote) {
    m_book_quote_data_store.Store(quote);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityBookQuote>& quotes) {
    m_book_quote_data_store.Store(quotes);
  }

  template<typename D>
  std::vector<SequencedTimeAndSale>
      CachedHistoricalDataStore<D>::load_time_and_sales(
        const SecurityMarketDataQuery& query) {
    return m_time_and_sale_data_store.Load(query);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const SequencedSecurityTimeAndSale& time_and_sale) {
    m_time_and_sale_data_store.Store(time_and_sale);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::store(
      const std::vector<SequencedSecurityTimeAndSale>& time_and_sales) {
    m_time_and_sale_data_store.Store(time_and_sales);
  }

  template<typename D>
  void CachedHistoricalDataStore<D>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_time_and_sale_data_store.Close();
    m_book_quote_data_store.Close();
    m_bbo_quote_data_store.Close();
    m_order_imbalance_data_store.Close();
    m_data_store->close();
    m_open_state.Close();
  }
}

#endif
